#  Devising wireless flood-sensing networks for critical infrastructure facilities


> **Intelligent Flood Monitoring System**: Integrating BIM data with genetic algorithm optimization for precise flood risk assessment in indoor environments

---

# 1 Summary of supplemental materials
This table below shows all supplemental materials. All sheets in Tables S1, S2, and S3 are arranged in the order shown in this table.

<img width="1436" height="999" alt="image" src="https://github.com/user-attachments/assets/8dc8e98e-cbb1-48a9-8b3f-0ee1db2d9e4b" />

# 2 General Introduction

2.1 This repository aims at providing the codes and data regarding the paper entitled “Devising wireless flood-sensing networks for critical infrastructure facilities” for the public, and it is developed by X, X, X, and X.

2.2 We greatly appreciate the selfless spirits of these voluntary contributors of a series of open python libraries, including ifcopenshell (https://github.com/stefkeB/ifcopenshell_examples), pyautocad (https://github.com/reclosedev/pyautocad), and so on. Our work stands on the shoulders of these giants.

2.3 As for anything regarding the copyright, please refer to the MIT License or contact the authors.


# 3 Methodology 
## 3.1 BIM-based wireless-related building information transformation 
The data transformation from BIM’s IFC format to Winprop’s IDA format aims to avoid manually and repeatedly establishing the building model for SCM wireless simulations. This process involves three main steps: (i) clarifying the data items related to SCM, (ii) devising the mapping schema between IFC and IDA, and (iii) developing the IFC2IDA data transformation function library. 


This library includes 18 functions to ensure the transformation of object attributes from IFC  to their counterparts in IDA.

![image](https://github.com/user-attachments/assets/a5222e03-a997-41ad-831d-39812c856e4a)


↑↑↑Codes for IFC2IDF function library

## 3.2 Development for SCM to optimize WANs

The dataset formulation consists of generating WPSs with the grid method and obtaining the energy consumption and natural lighting performance of corresponding WPSs.

This step involves designing the material library, mapping schema between IFC and IDA, and setting the key parameters of the SCM simulation
![image](https://github.com/user-attachments/assets/fdbc5d5b-5279-4412-9034-2054bb9c920c)



## 3.3 Optimization Results

3.3.1 Optimize WAN placements considering FDNs by GA
A tool for obtaining rainfall data of the simulation scenario through POT calculation
<img width="1265" height="723" alt="image" src="https://github.com/user-attachments/assets/b96dd3ae-0412-45c0-81b6-961a33b6e1ef" />


An algorithm based on GA has been devised for WAN placements optimization. The GA-based WAN considering FDNs optimization algorithm is structured into 10 steps. The coordinates of the WAN’s placements are skillfully transformed into a six-gene chromosome to represent a WAN node.
![image](https://github.com/user-attachments/assets/626efcca-ecf5-4f46-a6a8-cd2590f8c277)

↑↑↑Codes for NSGA-II-based WPS optimization algorithm

3.3.2 The performance comparison
7 schemes based on conventional approaches are developed for comparison, including: 
**Expert-based Approaches**: Huawei (SEbM1), Akuvox (SEbM2), Siemens (SEbM3)
**Numerical Optimization Methods**: Hexagonal deployment (SNO1), Circular deployment (SNO2)
**Traditional Methods**: FDN-unaware optimization (SFoU1)
![image](https://github.com/user-attachments/assets/58999a16-8ab2-45a7-ae2c-e8b1209ae92f)
<img width="1204" height="234" alt="image" src="https://github.com/user-attachments/assets/7180ed37-fa15-4d9b-840e-bfba34ddfa4d" />

# 4 Discussion
## 4.1 Case of educational facility
Signal values for the scheme from the Identification-Development-Optimization framework in the educational facility.
<img width="812" height="707" alt="image" src="https://github.com/user-attachments/assets/7ee8c4b7-93bf-4147-88a8-2e435ed08ba4" />

## 4.2 Capital and O&M cost in WFSN
The capital and O&M cost of the components of the wireless flood sensor network (WFSN). The O&M cost, including installation, configuration, communication setup, and maintenance, is generally estimated at about 50% of the total investment 
<img width="1393" height="641" alt="image" src="https://github.com/user-attachments/assets/6275f454-782e-44fa-8013-b0b786b45941" />

