let a = 0;
let b = 1;
let iterations = 10;

let i = 0;
while i < iterations {
	let tmp = b;
	let b = a + b;
	let a = tmp;
	
	let i = i + 1;
};
