# narom: auto MNA
Generate KCL system

## Rule
* node: a node is a vertex
* relation: a relation is an edge optional mounting a device 
* device: 
	* voltage: volt
	* current: ampere
	* resistor: ohm
	* capacitor: farad 

## Example 
	r1 1o
	r2 2o
	r3 3o
	c1 3f
	c2 2f
	c3 1f
	v1 1v
	n1 gnd  v1
	n2  n1  r1
	n2 gnd  c1
	n2  n3  r2
	n3 gnd  c2
	n3  n4  r3
	n4 gnd  c3
