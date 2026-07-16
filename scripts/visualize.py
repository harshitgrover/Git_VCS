#!/usr/bin/env python3
"""
MiniGit Object Visualizer
This script reads the .minigit/objects database, parses every object,
and generates an interactive HTML file with a flowchart of your
entire Directed Acyclic Graph (DAG) using Mermaid.js.
"""

import os
import zlib

def generate_html(gitdir):
    obj_dir = os.path.join(gitdir, 'objects')
    if not os.path.exists(obj_dir):
        print(f"Error: {obj_dir} does not exist.")
        return
        
    hashes = []
    for d in os.listdir(obj_dir):
        dp = os.path.join(obj_dir, d)
        if os.path.isdir(dp):
            for f in os.listdir(dp):
                hashes.append(d + f)
                
    nodes = {}
    for h in hashes:
        path = os.path.join(obj_dir, h[:2], h[2:])
        try:
            with open(path, 'rb') as f:
                decomp = zlib.decompress(f.read())
            
            null_idx = decomp.find(b'\0')
            if null_idx != -1:
                body = decomp[null_idx+1:]
                
                if body.startswith(b'tree '):
                    lines = body.split(b'\n')
                    tree_hash = lines[0][5:].decode()
                    parents = [l[7:].decode() for l in lines if l.startswith(b'parent ')]
                    nodes[h] = {'type': 'commit', 'tree': tree_hash, 'parents': parents}
                
                elif body.startswith(b'chunk '):
                    chunks = [l[6:].decode() for l in body.split(b'\n') if l.startswith(b'chunk ')]
                    nodes[h] = {'type': 'blob', 'chunks': chunks}
                
                else:
                    # Attempt to parse as Tree
                    entries = []
                    pos = 0
                    while pos < len(body):
                        space = body.find(b' ', pos)
                        if space == -1: break
                        null = body.find(b'\0', space)
                        if null == -1: break
                        if null + 1 + 64 <= len(body):
                            entries.append(body[null+1:null+1+64].decode())
                            pos = null + 1 + 64
                        else:
                            break
                    if entries:
                        nodes[h] = {'type': 'tree', 'entries': entries}
                    else:
                        nodes[h] = {'type': 'chunk'}
            else:
                nodes[h] = {'type': 'chunk'}
        except Exception as e:
            nodes[h] = {'type': 'chunk'}
            
    # Generate Mermaid Graph Syntax
    mermaid = "graph TD\n"
    
    mermaid += "classDef commit fill:#f9f,stroke:#333,stroke-width:2px;\n"
    mermaid += "classDef tree fill:#bbf,stroke:#333,stroke-width:2px;\n"
    mermaid += "classDef blob fill:#bfb,stroke:#333,stroke-width:2px;\n"
    mermaid += "classDef chunk fill:#ddd,stroke:#333,stroke-width:1px,stroke-dasharray: 5 5;\n\n"
    
    for h, data in nodes.items():
        short_h = h[:6]
        if data['type'] == 'commit':
            mermaid += f'    {h}["Commit<br>{short_h}"]:::commit\n'
            mermaid += f'    {h} -->|tree| {data["tree"]}\n'
            for p in data['parents']:
                mermaid += f'    {h} -->|parent| {p}\n'
        elif data['type'] == 'tree':
            mermaid += f'    {h}["Tree<br>{short_h}"]:::tree\n'
            for e in data['entries']:
                mermaid += f'    {h} -->|blob| {e}\n'
        elif data['type'] == 'blob':
            mermaid += f'    {h}["Blob Manifest<br>{short_h}"]:::blob\n'
            for c in data['chunks']:
                mermaid += f'    {h} -->|chunk| {c}\n'
        else:
            mermaid += f'    {h}["Data Chunk<br>{short_h}"]:::chunk\n'

    # Inject Branches
    mermaid += "classDef branch fill:#ffb,stroke:#333,stroke-width:2px,rx:10,ry:10;\n"
    mermaid += "classDef head fill:#fbb,stroke:#333,stroke-width:2px,rx:10,ry:10;\n"
    
    refs_dir = os.path.join(gitdir, 'refs', 'heads')
    if os.path.exists(refs_dir):
        for b in os.listdir(refs_dir):
            try:
                with open(os.path.join(refs_dir, b), 'r') as f:
                    c_hash = f.read().strip()
                if c_hash in nodes:
                    b_id = f"branch_{b}"
                    mermaid += f'    {b_id}["Branch: {b}"]:::branch\n'
                    mermaid += f'    {b_id} -.->|points to| {c_hash}\n'
            except Exception:
                pass
                
    head_file = os.path.join(gitdir, 'HEAD')
    if os.path.exists(head_file):
        try:
            with open(head_file, 'r') as f:
                ref_line = f.read().strip()
            if ref_line.startswith('ref: refs/heads/'):
                b_name = ref_line[16:]
                mermaid += f'    HEAD["HEAD"]:::head\n'
                mermaid += f'    HEAD -.->|active| branch_{b_name}\n'
            else:
                mermaid += f'    HEAD["HEAD (Detached)"]:::head\n'
                mermaid += f'    HEAD -.->|points to| {ref_line}\n'
        except Exception:
            pass

    html = f"""<!DOCTYPE html>
<html>
<head>
    <title>MiniGit DAG Visualizer</title>
    <script type="module">
      import mermaid from 'https://cdn.jsdelivr.net/npm/mermaid@10/dist/mermaid.esm.min.mjs';
      mermaid.initialize({{ startOnLoad: true }});
    </script>
    <style>
        body {{ font-family: sans-serif; text-align: center; background: #f4f4f9; padding: 20px; }}
        h2 {{ color: #333; }}
        .legend {{ margin: 20px auto; max-width: 600px; padding: 10px; background: white; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); display: flex; justify-content: space-around; }}
        .legend div {{ padding: 5px 15px; border-radius: 5px; font-weight: bold; border: 2px solid #333; }}
        .mermaid {{ display: flex; justify-content: center; margin-top: 30px; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); overflow-x: auto; }}
    </style>
</head>
<body>
    <h2>MiniGit Object Graph (DAG)</h2>
    
    <div class="legend">
        <div style="background: #f9f;">Commit</div>
        <div style="background: #bbf;">Tree</div>
        <div style="background: #bfb;">Blob Manifest</div>
        <div style="background: #ddd; border-style: dashed;">Data Chunk (CDC)</div>
        <div style="background: #ffb; border-radius: 10px;">Branch Pointer</div>
        <div style="background: #fbb; border-radius: 10px;">HEAD</div>
    </div>

    <div class="mermaid">
{mermaid}
    </div>
</body>
</html>"""

    with open('visualize.html', 'w') as f:
        f.write(html)
    abs_path = os.path.abspath('visualize.html')
    print("✨ Successfully generated visualize.html!")
    print(f"👉 Copy/paste this link into your web browser: file://{abs_path}")

if __name__ == '__main__':
    gitdir = '.minigit'
    if not os.path.exists(gitdir):
        print("❌ Error: Run this script from the root of a minigit repository (where .minigit exists).")
    else:
        generate_html(gitdir)
