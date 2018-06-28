let a = 0;
let b = 1;
let iterations = 10;

let i = 0;
while i < iterations {
	print a;
	
	let tmp = b;
	let b = a + b;
	let a = tmp;
	
	let i = i + 1;
};

print b;
