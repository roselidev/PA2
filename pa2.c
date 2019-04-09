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
  bool minus = false;

  //check sign
  if(f>>31==1){
    minus=true;
    f = ~f+1;
  }

  //
  //Find if the value infinite or zero and if not, Get E value
  //rough normalization

  int tmp = x<<10;
  int E;
  //    infinite zone
  if(tmp>=16){
    ans = ans | 0b01110000;return ans;
  }
  //    normalize zone
  else if(tmp>=8)E=3;
  else if(tmp>=4)E=2;
  else if(tmp>=2)E=1;
  else if(tmp==1)E=0;
  else if(x>=1<<9)E=-1;
  else if(x>=1<<8)E=-2;

  //    denormalize zone
  else if(x>=1<<3)E=-3;
  else{ans= ans & 0b00000000;return ans;}//set as +0.0

  //
  //Rounding
  //
 
  //  get GRS bit values
  unsigned int Gbit = x & 1<<(6+E);
  unsigned int Rbit = x & 1<<(5+E);
  unsigned int Sbit = x & (1<<(5+E)-1);
  
  //  Round up condition
  if(Rbit==1 && Sbit!=0 || Gbit==1 && Rbit==1 && Sbit==0){
    x += 1<<(6+E);
    if(x & (1<<(10+E))) E++;//If carry occurs, renormalize
    if(E>3){ans = ans | 0b01110000; return ans;}//check infinity
  }

  //
  //Assign exp bits and frac bits to answer
  //
  
  unsigned int exp = (E+BIAS)<<4;
  ans += exp;
  unsigned int frac = (x & (1<<(9+E) + 1<<(8+E) + 1<<(7+E) + 1<<(6+E)))>>(6+E);
  ans += frac;

  //check sign bit
  if(minus==true){
    ans += 1<<31;
  }

  return ans;
}


fixp tinyfp2fixp(tinyfp t)
{
  //declare answer
  fixp ans=0x00000000;
  bool minus = false;

  //check sign bit
  if(t>>7==1){
    minus = true;
    t-=1>>7;
  }

  //when t is zero or infinity or NaN
  if(t == 0b00000000){
    ans = 0x00000000;return ans;
  }
  else if(t>>4 == 0b0111){
    ans = 0x80000000;return ans;
  }
  //when t is normalized / denormalized non-zero value
  else{
    if(t>>4==0b0000){
      t<<3;
      ans = ans + t;
    }
    else{
      int E = (t>>4) - BIAS;
      ans = ans + t<<(5+E);
    }

    //check sign bit
    if(minus == true){
      ans = ~ans+1;
      return ans;
    }
    return ans;
  }

	return 9;
}


tinyfp float2tinyfp(float f)
{



	return 9;
}


float tinyfp2float(tinyfp t)
{



	return 9.9;
}
