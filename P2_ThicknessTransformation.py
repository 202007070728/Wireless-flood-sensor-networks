import ifcopenshell

def ThicknessTransformation(ifc_file_path):

    ifc_file = ifcopenshell.open(ifc_file_path)
    extracted_thickness = None

    for property in ifc_file.by_type('IfcPropertySingleValue'):
        if property.Name == 'Thickness':
            extracted_thickness = property.NominalValue.wrappedValue if property.NominalValue else 'None'
            break

    if extracted_thickness is None:
        raise ValueError("Failed to extract the Thickness property value from the IFC file")


# 调用函数，传入IFC文件的路径
ifc_file_path = r'path of ifc file'
ThicknessTransformation(ifc_file_path)