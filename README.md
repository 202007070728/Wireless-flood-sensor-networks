#  Devising wireless flood-sensing networks for critical infrastructure facilities


> **Intelligent Flood Monitoring System**: Integrating BIM data with genetic algorithm optimization for precise flood risk assessment in indoor environments

---

# 1 Summary of supplemental materials
This table below shows all supplemental materials. All sheets in Tables S1, S2, and S3 are arranged in the order shown in this table.


# 2 General Introduction

2.1 This repository aims at providing the codes and data regarding the paper entitled “Devising wireless flood-sensing networks for critical infrastructure facilities” for the public, and it is developed by X, X, X, and X.

2.2 We greatly appreciate the selfless spirits of these voluntary contributors of a series of open python libraries, including ifcopenshell (https://github.com/stefkeB/ifcopenshell_examples), pyautocad (https://github.com/reclosedev/pyautocad), and so on. Our work stands on the shoulders of these giants.

2.3 As for anything regarding the copyright, please refer to the MIT License or contact the authors.


# 3 Methodology 
## 3.1 BIM-based wireless-related building information transformation 
The data transformation from BIM’s IFC format to Winprop’s IDA format aims to avoid manually and repeatedly establishing the building model for SCM wireless simulations. This process involves three main steps: (i) clarifying the data items related to SCM, (ii) devising the mapping schema between IFC and IDA, and (iii) developing the IFC2IDA data transformation function library. 


This library includes 18 functions to ensure the transformation of object attributes from IFC  to their counterparts in IDA.


![image](Image/IFC2IDF.png)
↑↑↑Codes for IFC2IDF function library

## 3.2 Development for SCM to optimize WANs

The dataset formulation consists of generating WPSs with the grid method and obtaining the energy consumption and natural lighting performance of corresponding WPSs.

This step involves designing the material library, mapping schema between IFC and IDA, and setting the key parameters of the SCM simulation


## 3.3 Optimization Results

3.3.1 Optimize WAN placements considering FDNs by GA
An algorithm based on GA has been devised for WAN placements optimization. The GA-based WAN considering FDNs optimization algorithm is structured into 10 steps. The coordinates of the WAN’s placements are skillfully transformed into a six-gene chromosome to represent a WAN node.

↑↑↑Codes for NSGA-II-based WPS optimization algorithm

3.3.2 The performance comparison
7 schemes based on conventional approaches are developed for comparison, including: 
**Expert-based Approaches**: Huawei (SEbM1), Akuvox (SEbM2), Siemens (SEbM3)
**Numerical Optimization Methods**: Hexagonal deployment (SNO1), Circular deployment (SNO2)
**Traditional Methods**: FDN-unaware optimization (SFoU1)