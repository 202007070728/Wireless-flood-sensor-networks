import ifcopenshell
import ifcopenshell.geom
import ifcopenshell.util.shape
import numpy as np

# 定义一个函数将立体构件分面
def group_points_by_axis(verts, axis, value):
    return [point for point in verts if point[axis] == value]

# 定义一个函数使得每面坐标按照顺时针/逆时针输出
def sort_group(group):
    def only_one_difference(point1, point2):
        differences = 0
        for i in range(3):
            if point1[i] != point2[i]:
                differences += 1
        return differences == 1
    base_point = group[0]
    sorted_group = [base_point]
    while len(group) > 1:
        next_point = None
        for point in group:
            if point not in sorted_group and only_one_difference(sorted_group[-1], point):
                if not next_point or only_one_difference(sorted_group[-1], next_point) > only_one_difference(sorted_group[-1], point):
                    next_point = point
        if next_point:
            sorted_group.append(next_point)
            group.remove(next_point)
        else:
            sorted_group.append(group.pop())
    return sorted_group

# 将每一个立体墙进行分面输出
def sort_and_print_points(verts):
    for axis in range(3):  # 0: x, 1: y, 2: z
        for value in set(point[axis] for point in verts):
            group = group_points_by_axis(verts, axis, value)
            sorted_group = sort_group(group)
            #print(f"Group by axis {axis} and value {value}:")#如果需要输出
            for point in sorted_group:
                return(point)
            return()

# 更新IDA文件中的墙坐标
def update_vertex_in_idf(ida_file_path, coordinates):
    with open(ida_file_path, 'r') as file:
        lines = file.readlines()

    # 用于存储更新后的内容
    updated_lines = []

    # 标记是否到达墙体坐标部分
    update_coords = False
    slab_index = 0

    for line in lines:
        if 'BEGIN_WALLS' in line:
            update_coords = True
            updated_lines.append(line)
            continue
        if 'END_WALLS' in line:
            update_coords = False
            updated_lines.append(line)
            continue
        parts = line.split()
        coords = coordinates
        coord_str = ' '.join(
            f'{coords[0]}, {coords[1]}, {coords[2]},{coords[3]}')
        rest = ' '.join(parts[4:])  # 保留除坐标外的其他信息
        updated_lines.append(f'{slab_index} 4 {coord_str} {rest}\n')
        print("已更新")

    else:
        updated_lines.append(line)
        print("未更新")
        with open(ida_file_path, 'w') as file:
            file.writelines(lines)

#获得IFC文件中的坐标并输出为IDA文件
def Get_slab_vertices_world_coordinates(ifc_file_path,ida_file_path):
    """
    Load an IFC file and extract world coordinates of all slab vertices.

    :param ifc_file_path: Path to the IFC file
    :return: A list of tuples, where each tuple contains the slab index and its vertices in world coordinates
    """
    # Load the IFC file
    ifc_file = ifcopenshell.open(ifc_file_path)

    with open(ida_file_path, 'r') as file:
        lines = file.readlines()

    # 用于存储更新后的内容
    updated_lines = []

    # 标记是否到达墙体坐标部分
    update_coords = False
    slab_index = 0

    for line in lines:
        if 'BEGIN_WALLS' in line:
            update_coords = True
            updated_lines.append(line)
            continue
        if 'END_WALLS' in line:
            update_coords = False
            updated_lines.append(line)
            continue
    # Select all slab entities
    slabs = ifc_file.by_type('IfcSlab')

    # Prepare a list to hold all the vertices in world coordinates
    world_vertices = []
    points_by_slab = {}
    # Iterate through each slab to get its absolute coordinates
    for i, slab in enumerate(slabs):
        settings = ifcopenshell.geom.settings()
        shape = ifcopenshell.geom.create_shape(settings, slab)

        # Get the transformation matrix for the shape
        matrix = ifcopenshell.util.shape.get_shape_matrix(shape)

        # Apply the transformation matrix to each vertex
        verts = shape.geometry.verts
        slab_vertices = []
        for j in range(0, len(verts), 3):
            # Extract the x, y, z coordinates of the vertex
            x, y, z = verts[j], verts[j + 1], verts[j + 2]

            # Convert the vertex to homogeneous coordinates (w=1)
            homogeneous_vert = np.array([x, y, z, 1.0])

            # Apply the transformation matrix
            transformed_vert = np.dot(matrix, homogeneous_vert)

            # Round the transformed vertex to one decimal place and append to the list (excluding the w component)
            slab_vertices.append(np.round(transformed_vert[:3], 1).tolist())

        # Now slab_vertices contains all the vertices in world coordinates for this slab
        points_by_slab[i + 1] = slab_vertices

    # Print the points for each slab
    for slab_index, vertices in points_by_slab.items():
        print(f"墙{slab_index}的顶点坐标：")
        # for vertex in vertices:
        # print(vertex)
        print(points_by_slab[slab_index])  # Add an empty line for better readability between slabs
        for axis in range(3):  # 0: x, 1: y, 2: z        for value in set(point[axis] for point in points_by_window[window_index]):
            for value in set(point[axis] for point in points_by_slab[slab_index]):
                group = group_points_by_axis(points_by_slab[slab_index], axis, value)
                sorted_group = sort_group(group)
                update_vertex_in_idf(ida_file_path, sorted_group)

# 调用主函数
ifc_file_path = r'path of ifc file'
ida_file_path = r'path of idf file'
Get_slab_vertices_world_coordinates(ifc_file_path,ida_file_path)
