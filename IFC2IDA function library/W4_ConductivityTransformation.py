import ifcopenshell

def Extract_Conductivity_value_from_materials_and_finishes(ifc_file_path):
    # 打开IFC文件
    ifc_file = ifcopenshell.open(ifc_file_path)

    # 获取所有的材料属性信息
    material_properties = ifc_file.by_type('IfcMaterialProperties')

    # 遍历材料属性信息
    for material_property in material_properties:
        # 获取材料名称
        material_name = material_property.Name
        if material_name == 'Pest_WirelessPropagation':
            # 获取材料的属性信息
            properties = material_property.Properties

            # 遍历每个材料的属性信息
            for prop in properties:
                param_name = prop.Name
                param_value = prop.NominalValue.wrappedValue
                if param_name == 'Conductivity':
                    print(f"Parameter: {param_name}, Value: {param_value}")
                    return param_value  # 返回Conductivity的值

# 调用函数，传入IFC文件的路径
ifc_file_path = r'path of ifc file'
Conductivity_value = Extract_Conductivity_value_from_materials_and_finishes(ifc_file_path)
if Conductivity_value is not None:
    print("Conductivity value extracted:", Conductivity_value)
else:
    print("No Conductivity value found for 'Pest_WirelessPropagation'.")
