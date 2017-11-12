void main(){
	char* xx= (char*)0xb8000;
	char* msg = "HELLO WORLD FROM C";
	char deze;
	int i = 0;
	int z = 0;
	while((deze = msg[i++])!=0x00){
		xx[z++] = deze;
		xx[z++] = 0x70;
	}
	kernel_print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
}
