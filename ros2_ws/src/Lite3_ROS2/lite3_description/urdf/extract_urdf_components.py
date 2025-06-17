  
# file for extract data from urdf file 

import xml.etree.ElementTree as ET

# Load the URDF file
urdf_file = 'robot.urdf'  # Replace with your actual URDF filename
tree = ET.parse(urdf_file)
root = tree.getroot()

output_lines = []

# Extract Links
links = [link.get('name') for link in root.findall('link')]
output_lines.append("Links:")
output_lines.extend(links)
output_lines.append("")

# Extract Joints
output_lines.append("Joints:")
for joint in root.findall('joint'):
    joint_info = {
        'name': joint.get('name'),
        'type': joint.get('type'),
        'parent': joint.find('parent').get('link'),
        'child': joint.find('child').get('link')
    }
    output_lines.append(str(joint_info))
output_lines.append("")

# Extract Meshes
output_lines.append("Meshes:")
for link in root.findall('link'):
    visual = link.find('visual')
    if visual is not None:
        geometry = visual.find('geometry')
        if geometry is not None:
            mesh = geometry.find('mesh')
            if mesh is not None:
                mesh_info = {
                    'link': link.get('name'),
                    'mesh_file': mesh.get('filename')
                }
                output_lines.append(str(mesh_info))
output_lines.append("")

# Extract Materials
output_lines.append("Materials:")
for material in root.findall('material'):
    color = material.find('color')
    material_info = {
        'name': material.get('name'),
        'rgba': color.get('rgba') if color is not None else None
    }
    output_lines.append(str(material_info))
output_lines.append("")

# Save to a text file
output_file = 'urdf_components_output.txt'
with open(output_file, 'w') as f:
    f.write('\n'.join(output_lines))

print(f"Extraction complete! Results saved in '{output_file}'")
