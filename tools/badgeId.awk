#!/bin/gawk -f

# badgeId.awk
# hacked from: chkihex.awk
# check intel-hex file
# 2012/10/09 K.Takesita

# hacked by peb 2015
# rehacked 2019/05/20

# usage:
# badgeId.awk badgeId=010F

# note: use hex with zero padding for 4 digits
#   basically have to find the 0xdead value in hex
#   search this file for tag 'dead'
#   and change the start byte to match where you found.
#   Hex may have multiple "dead" values, in that case find the
#   one with the ascii value of the hextab[] string
#   which precedes "finalBadgeId" (lines 32-35 of badge.c)
#
# Intel HEX format
# http://en.wikipedia.org/wiki/Intel_HEX
#
# :bbaaaarrdddd..ddcc
# bb Bytecount
# aaaa Address
# rr Recordtype
# dd.. Data
# cc Checksum
#
BEGIN{
	H2D[0]=0;H2D[1]=1;H2D[2]=2;H2D[3]=3;
	H2D[4]=4;H2D[5]=5;H2D[6]=6;H2D[7]=7;
	H2D[8]=8;H2D[9]=9;
	H2D["A"]=10;H2D["B"]=11;H2D["C"]=12;H2D["D"]=13;H2D["E"]=14;H2D["F"]=15;
	H2D["a"]=10;H2D["b"]=11;H2D["c"]=12;H2D["d"]=13;H2D["e"]=14;H2D["f"]=15;

	D2H[0]=0;D2H[1]=1;D2H[2]=2;D2H[3]=3;
	D2H[4]=4;D2H[5]=5;D2H[6]=6;D2H[7]=7;
	D2H[8]=8;D2H[9]=9;
	D2H[10]="a";D2H[11]="b";D2H[12]="c";D2H[13]="d";D2H[14]="e";D2H[15]="f";

}
#
$0 !~/^:[0-9A-Fa-f]+$/ { # unmatch
	printf("%d ; Unmatch Intel-HEX format\n",NR); print $0;
	next;
}
{
        hexLo=substr(badgeId,1,2);
        hexHi=substr(badgeId,3,2);

	Bytecount=substr($0,2,2);
	Address=substr($0,4,4);
	AddressH=substr(Address,1,2);
	AddressL=substr(Address,3,2);
	Recordtype=substr($0,8,2);
	Datalength=hex2dec(Bytecount);

	if (length(Recordtype)<2) {
		printf("%d ; Recordtype too short\n",NR); print $0;
		next;
	}
	if (length(Address)<4) {
		printf("%d ; Address too short\n",NR); print $0;
		next;
	}
	if (length(Bytecount)<2) {
		printf("%d ; Bytecount too short\n",NR); print $0;
		next;
	}

	sum = (Datalength + hex2dec(AddressH) + hex2dec(AddressL) + hex2dec(Recordtype))%256;

	if (Recordtype=="05") {
		#printf("%d ; SLA\n",NR); print $0;
		# NOCHECK
		print $0;
		next;
	}
	if (Recordtype=="04") {
		#printf("%d ; ELA\n",NR); print $0;
		# NOCHECK
		print $0;
		next;
	}
	if (Recordtype=="03") {
		#printf("%d ; SSA\n",NR); print $0;
		# NOCHECK
		print $0;
		next;
	}
	if (Recordtype=="02") {
		#printf("%d ; ESA\n",NR); print $0;
		# NOCHECK
		print $0;
		next;
	}
	if (Recordtype=="01") {	# EOF
		#printf("%d ; EOF address= %s\n",NR,Address);
		if (length($0)<11) {
			printf("%d ; EOF record too short\n",NR); print $0;
			next;
		}
		if (length($0)>11) {
			printf("%d ; EOF record too long\n",NR); print $0;
			next;
		}
		Checksum=substr($0,10,2);
		if (hex2dec(Checksum)!=compl2(sum)) {
			printf("%d ; EOF Checksum unmatch\n",NR); print $0;
			printf("checksum %s %d\n",hex2dec(Checksum),compl2(sum) ); print $0;
			next;
		}
		print $0;
		next;
		}
	if (Recordtype!="00") {
		#printf("%d ; Illegal Recordtype\n",NR); print $0;
		next;
	}

	if (Datalength!=16 && Datalength!=32) {
		#printf("%d ; Warning, Usual Bytecount?\n",NR); print $0;
	}

	printf(":" Bytecount Address Recordtype);
	flag1 = "";
	flag2 = "";
	mysum = 0;
	# check data
	for(i=0;i<Datalength;i++) {
		d=substr($0,10+i*2,2);
		if (length(d)<2) {
			printf("%d ; Data too short\n",NR); print $0;
			next;
		}

                # vim hex, find 'dead'
                # i==4 bcs thats where first 'de' part of hex pair started in hex
		# :08c0500043444546dead00004b
		#          ^byte 1 ^byte 4
		if (i == 4) { 
			flag1 = d;
			mySum = (sum + hex2dec(hexHi))%256; # our sum if we intercept
		}
		sum = (sum + hex2dec(d))%256;

                # PEB- find our flag data.
		if (i == 5) { 
		    if (flag1 == "de") {
			flag2 = d;
		        if (flag2 == "ad") { #replace checksum with ours
			    sum = (mySum + hex2dec(hexLo))%256; # our sum if we intercept
			    printf(hexHi);
			    printf(hexLo);
			}
			else {
		            printf(flag1);
			    printf(flag2);
			}
		    }
		    else {
		        printf(flag1);
		        printf(d);
		    }
		}

		if (i < 4) { 
		        printf(d);
		}
		if (i > 5) { 
		        printf(d);
		}
	}
	printf( dec2hex(compl2(sum)) "\n" );
	#outdata = (hexHi hexLo);
	#printf("%d ; outdata hexHi %s hexLo %s \n", NR, hexHi, hexLo);
	#printf("outdata\n", NR, outdata);


	#if (length(Checksum)<2) {
	#	printf("%d ; Checksum too short\n",NR); print $0;
	#	next;
	#}
	#if (length($0)>11+Datalength*2) {
	#	printf("%d ; Record too long\n",NR); print $0;
	#	next;
	#}
#	if (hex2dec(Checksum)!=compl2(sum)) {
#		printf("%d ; Checksum unmatch\n",NR); print $0;
#		printf( ":" Bytecount Address Recordtype hexHi hexLo dec2hex(compl2(sum)) "\n" );
#		printf("Record sum= %02X ,expected value= %s\n",sum,hex2dec(Checksum));
#		next;
#	}

	# passed.
}
END{
	#print "END." ;
}
#
func chkrtype(r ,rr) {
	rr=r+0;
	if (rr>5) rr=-1;
	return rr;
}
#
func compl2(num ,t) {
	t=num%256;
	return t?256-t:0;
}

func hex2dec(str ,i,a) {
	a=0;
	for(i=1;i<=length(str);i++) a= a*16 + H2D[substr(str,i,1)];
	return a;
}
func dec2hex(d, d0, d1, o) {
        d0 = D2H[rshift(d, 4)];
        d1 = D2H[and(d, 15)];
        o = (d0 d1)
	
	return o;
}
