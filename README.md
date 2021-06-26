# narom: auto MNA
Generate KCL system

## Rule
* node: a node is a vertex
* relation: a relation is an edge optionally mounting a device 
* device: 
	* voltage: volt
	* current: ampere
	* conductor: siemens
	* capacitor: farad 

## Example 
![image](https://github.com/superpi15/narom/blob/main/rc.png)

	.susceptance C
	.conductance G
	.excitation  E
	g1 1s
	g2 1/2s
	g3 1/3s
	g4 1/4s
	c1 3f
	c2 2f
	c3 1f
	v1 1v
	n1 gnd  v1
	n2  n1  g1
	n2 gnd  c1
	n2  n3  g2
	n3 gnd  c2
	n3  n4  g3
	n5  n4  c3
	n5 gnd  g4
## Result
Terminal:

	building mna ... 
	  0: n1
	  1: n2
	  2: n3
	  3: n4
	  4: n5
	  5: 	n1 (+  v1  -) gnd
	susceptance matrix:
	          0          0          0          0          0          0
	          0         c1          0          0          0          0
	          0          0         c2          0          0          0
	          0          0          0         c3        -c3          0
	          0          0          0        -c3         c3          0
	          0          0          0          0          0          0
	conductance matrix: 
	         g1        -g1          0          0          0         -1
	        -g1      g1+g2        -g2          0          0          0
	          0        -g2      g2+g3        -g3          0          0
	          0          0        -g3         g3          0          0
	          0          0          0          0         g4          0
	          1          0          0          0          0          0
	excitation vector: 
	          0          0          0          0          0         v1

File: 

	% This matlab code is generated by 'Narom - auto MNA' 
	 
	C=zeros(6,6);
	G=zeros(6,6);
	E=zeros(6,1);
	g1= 1 ;
	g2= 1/2 ;
	g3= 1/3 ;
	g4= 1/4 ;
	c1= 3 ;
	c2= 2 ;
	c3= 1 ;
	v1= 1 ;
	C(2,2)=  c1;
	C(3,3)=  c2;
	C(4,4)=  c3;
	C(4,5)= -c3;
	C(5,4)= -c3;
	C(5,5)=  c3;
	G(1,1)=  g1;
	G(1,2)= -g1;
	G(1,6)= -1;
	G(2,1)= -g1;
	G(2,2)=  g1+g2;
	G(2,3)= -g2;
	G(3,2)= -g2;
	G(3,3)=  g2+g3;
	G(3,4)= -g3;
	G(4,3)= -g3;
	G(4,4)=  g3;
	G(5,5)=  g4;
	G(6,1)= 1;
	E(6)=  v1;
