//--------------------------------------------------------------
// 
//  4190.308 Computer Architecture (Spring 2019)
//
//  Project #2: TinyFP Representation
//
//  April 4, 2019
//
//  Jin-Soo Kim (jinsoo.kim@snu.ac.kr)
//  Systems Software & Architecture Laboratory
//  Dept. of Computer Science and Engineering
//  Seoul National University
//
//--------------------------------------------------------------

#include <stdio.h>
#include "pa2.h"
#define BIAS 3

tinyfp fixp2tinyfp(fixp f)
{
  //declare answer
  tinyfp ans=0b00000000;
  int minus = 0;
  int denorm=0;

  //check sign
  if((f>>31)&1){
    minus=1;
    f = ~f+1;
  }

  //
  //Find if the value infinite or zero and if not, Get E value
  //rough normalization

  int tmp = f>>10;
  int E=0;
  //    infinite zone
  if(tmp>=16){
    ans = 0b1110000;
    if(minus){ans+=(1<<7);}
    return ans;
  }
  //    normalize zone
  else if(tmp>=8)E=3;
  else if(tmp>=4)E=2;
  else if(tmp>=2)E=1;
  else if(tmp==1)E=0;
  else if(((int)f)>=(1<<9))E=-1;
  else if(((int)f)>=(1<<8))E=-2;


  //    denormalize zone
  else if(((int)f)>=(1<<3)){E=-2;denorm=1;}
  else{ans= 0b00000000;if(minus){ans+=(1<<7);}return ans;}//set as 0


  //
  //Rounding
  //
 
  //  get GRS bit values
  int GbitIndex = (6+E);
  int Gbit = f & 1<<GbitIndex;
  int Rbit = f & 1<<(GbitIndex-1);
  int Sbit = f & ((1<<(GbitIndex-1))-1);
  //  Round up condition
  if((Rbit!=0 && Sbit!=0) || (Gbit!=0 && Rbit!=0 && Sbit==0)){
    f += 1<<(GbitIndex);
    if(denorm){E--;}
    if(f & (1<<(11+E))){E++;if(denorm){denorm=0;GbitIndex--;}GbitIndex++;}//If carry occurs, renormalize
    if(E>3){ans = 0b01110000; if(minus) ans+=(1<<7); return ans;}//check infinity
  }

  //
  //Assign exp bits and frac bits to answer
  //
  
  int exp = (E+BIAS)<<4;
  if(denorm==1){exp=0;}
  ans += exp;
  int frac = (f & ((1<<(GbitIndex+3)) + (1<<(GbitIndex+2)) + (1<<(GbitIndex+1)) + (1<<(GbitIndex))));
  frac = frac>>(GbitIndex);
  ans += frac;

  //check sign bit
  if(minus==1){
    ans += (1<<7);
  }

  return ans;
}


fixp tinyfp2fixp(tinyfp t)
{
  //declare answer
  fixp ans=0x00000000;
  int minus = 0;

  //check sign bit
  if((t>>7)==1){
    minus = 1;
    t-=(1<<7);
  }

  //when t is zero or infinity or NaN
  if(t & 0b00000000){
    ans = 0x00000000;return ans;
  }
  else if(t>>4 == 0b0111){
    ans = 0x80000000;return ans;
  }
  //when t is normalized / denormalized non-zero value
  else{
    if((t>>4)==0b0000){
      ans = ans + (t<<4);
    }
    else{
      int E = (t>>4) - BIAS;
      t = t & 0b0001111;//remove all exp part and make normalize form
      t += 1<<4;
      ans = ans + (t<<(6+E));
    }

    //check sign bit
    if(minus){
      ans = ~ans+1;
      return ans;
    }
    return ans;
  }

	return 9;
}

union bitfloat{
  float flo;
  int bit;
};

tinyfp float2tinyfp(float f)
{
  //declare answer
  tinyfp ans=0b00000000;
  int minus=0;
  union bitfloat F;
  F.flo=f;
  int frac = F.bit & 0x007fffff;

  //check sign value
  if(F.bit>>31){
    minus=1;
    F.bit-=(1<<31);
  }
  //check infinity / NaN
  if((F.bit>>23)==0b011111111){
    if(frac==0){//infinity
      ans=0b01110000;
      if(minus){ans+=(1<<7);}
      return ans;
    }
    else{//NaN
      ans=0b01111111;
      if(minus){ans+=(1<<7);}
      return ans;
    }
  }


  //Rounding
  int Gbitindex=19;
  int Gbit= (frac>>Gbitindex) & 1;
  int Rbit= (frac>>(Gbitindex-1)) & 1;
  int Sbit= (frac & (0x0003ffff));
  int exp =(F.bit>>(Gbitindex+4));
    
  if((Rbit!=0 && Sbit!=0) || (Gbit!=0 && Rbit!=0 && Sbit==0)){
    F.bit += (1<<Gbitindex);}
  int newExp=(F.bit>>(Gbitindex+4));
  if(newExp!=exp)Gbitindex++;
  

  //trim out invalid exp range
  exp=newExp;
  if(exp>130){//infinity
    ans=0b01110000;
    if(minus){ans+=(1<<7);}
    return ans;
  }
  if(exp<=119){//zero
    ans=0b00000000;
    if(minus){ans+=(1<<7);}
    return ans;
  }

  //norm to denorm 
  if(exp<=124){
    frac += (1<<23);
    //round again
    Gbitindex += (125-exp);//-2-(exp-127)
    Gbit= (frac>>Gbitindex)&1;
    Rbit= (frac>>(Gbitindex-1))&1;
    Sbit= frac & ((1<<(Gbitindex-1))-1);

    if((Rbit != 0 && Sbit !=0) || (Gbit!=0 && Rbit!=0 && Sbit==0)){
      frac+=(1<<Gbitindex);
      if(exp==124 && (frac & (1<<(Gbitindex+4)))){
        ans+=(1<<4);
      }
    }

    for(int i=3;i>-1;i--){
      ans+=((frac>>(Gbitindex+i))&1)<<i;
    }
    if(minus){ans+=(1<<7);}
    return ans;
  }

  //norm to norm

  ans+=((exp-124)<<4);
  ans += (((F.bit>>(Gbitindex+3))&1)<<3)+(((F.bit>>(Gbitindex+2))&1)<<2)+(((F.bit>>(Gbitindex+1))&1)<<1)+((F.bit>>Gbitindex)&1);
 


  if(minus){ans+=(1<<7);}
  return ans;
}


float tinyfp2float(tinyfp t)
{
  //declare values
  union bitfloat ans;
  ans.bit = 0x00000000;
  int minus=0;

  //check sign
  if((t>>7)){
    minus=1;
    t-=(1<<7);
  }

  //check infinity / NaN
  if((t>>4)==0b0111){
    int fflag = t & 0b00001111;
    if(fflag==0){//infinity
      ans.bit=0x7f800000;
      if(minus){ans.bit+=(1<<31);}
      return ans.flo;
    }
    else{//NaN
      ans.bit=0x7fffffff;
      if(minus){ans.bit+=(1<<31);}
      return ans.flo;
    }
  }

  //copy all binary value in t to the ans
  int exp = (t>>4)+124;
  int frac = t & 0b00001111;
  int Gbit = 19;
  if(exp==124 && frac==0){if(minus){ans.bit+=(1<<31);}return ans.flo;}//zero
  if(exp==124 && frac!=0){//denormalized value
    if(frac>=8){
      frac-=8;
      Gbit++;
    }
    else if(frac>=4){
      frac-=4;
      Gbit+=2;
      exp--;
    }
    else if(frac>=2){
      frac-=2;
      Gbit+=3;
      exp-=2;
    }
    else if(frac==1){
      frac-=1;
      Gbit+=4;
      exp-=3;
    }
  }
  ans.bit += (exp<<23);
  ans.bit += frac<<Gbit;

  if(minus){ans.bit+=(1<<31);}
  return ans.flo;
}
