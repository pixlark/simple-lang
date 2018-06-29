let fizz = 4122;
let buzz = 3022;

let i = 1;

while i <= 100 {
	if i % 3 == 0 {
		print fizz;
	} else {
		if i % 5 == 0 {
			print buzz;
		} else {
			print i;
		};
	};
	let i = i + 1;
};
