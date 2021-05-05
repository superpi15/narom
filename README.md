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
