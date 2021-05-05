# narom: auto MNA
Generate KCL system

## Rule
* node: a node is a vertex
* relation: a relation is an edge optional mounting a device 
* device: 
	* voltage: volt
	* current: ampere
	* conductor: siemens
	* capacitor: farad 

## Example 
![image](https://github.com/superpi15/narom/blob/main/rc.png)

	g1 1s
	g2 0.5s
	g3 0.33s
	g4 0.25s
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
