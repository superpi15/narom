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
	g2 2s
	g3 3s
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
	n4 gnd  c3
## Result
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
