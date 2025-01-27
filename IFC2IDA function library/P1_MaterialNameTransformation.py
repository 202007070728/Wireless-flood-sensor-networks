import ifcopenshell


def MaterialNameTransformation(ifc_file_path):
    ifc_file = ifcopenshell.open(ifc_file_path)
    extracted_name = None

    for property in ifc_file.by_type('IfcPropertySingleValue'):
        if property.Name == 'Name':
            extracted_name = property.NominalValue.wrappedValue if property.NominalValue else 'None'
            print("Material Name:", extracted_name)
            break

    if extracted_name is None:
        raise ValueError("Failed to extract the Name property value from the IFC file")

# 调用函数，传入IFC文件的路径
ifc_file_path = r'path of ifc file'
MaterialNameTransformation(ifc_file_path)
