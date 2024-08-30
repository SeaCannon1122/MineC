
clamp_int(int val, int min, int max) {
	return (val < min ? min : (val > max ? max : val));
}