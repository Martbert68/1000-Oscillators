/* grand.c...
mover.c ran out of CPU and memory with its crazy inner loop so this is a freshening and rethinking of that.

Going to add multi point and variable timing reverb in a pointer to a 2 hour long array.
Going to just step along this huge array to reduce all the testing to see if Im at the end. I have RAM but not Bandwidth
Going to get rid of all these globals.
I mean looking up a in a global array in another global...in the inner loop. PLEASE
				f=notes[tnote[s]];
The UI is pants but can stay for now. It's main_osc that needs help.

*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


// HOW BIG IS THE BUFFER? 2 Hours at 44100 * 60 *2
#define SMAX 2*44100*60*2 


static float notes[108]={
16.35,17.32,18.35,19.45,20.60,21.83,23.12,24.50,25.96,27.50,29.14,30.87,
32.70,34.65,36.71,38.89,41.20,43.65,46.25,49.00,51.91,55.00,58.27,61.74,
65.41,69.30,73.42,77.78,82.41,87.31,92.50,98.00,103.8,110.0,116.5,123.5,
130.8,138.6,146.8,155.6,164.8,174.6,185.0,196.0,207.7,220.0,233.1,246.9,
261.6,277.2,293.7,311.1,329.6,349.2,370.0,392.0,415.3,440.0,466.2,493.9,
523.3,554.4,587.3,622.3,659.3,698.0,740.0,784.0,830.6,880.0,932.3,987.8,
1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951,
4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902 }; 


void usage ()
{
	printf("usage: mover\n");
	exit (1);
}

int main(int argc,char *argv[])
{
 
	int *fhead,len,chan,sample_rate,bits_pers,byte_rate,ba,size;
	int number,along,osc,note;
	short *waveform;


	number=1000;

	double fpoint[1000];
	double fvar[1000];
	double fcut[1000];


        fhead=(int *)malloc(sizeof(int)*11);

        len=60;
        chan=2;
        sample_rate=44100;
        bits_pers=16;
        byte_rate=(sample_rate*chan*bits_pers)/8;
        ba=((chan*bits_pers)/8)+bits_pers*65536;
        size=chan*len*sample_rate;
        waveform=(short *)malloc(sizeof(short)*size);

	// setup our oscillatorsi drift up to 3%. 
	for (osc=0;osc<number;osc++){ fvar[osc]=1+(double)(rand()%30000)/1000000;}
	for (osc=0;osc<number;osc++){ fcut[osc]=0.5+(double)(rand()%32768)/32768;}

	for (along=0;along<size;along+=2)
	{
		long left,right;
		left=0;right=0;
		note=(108*along)/size;
		//note=48+((8*along)/size);
		for (osc=0;osc<number;osc+=2)
		{
			int o_left,o_right;				
			float lf,rf;

			fpoint[osc]+=(2*M_PI*(notes[note]*fvar[osc]))/sample_rate;
			fpoint[osc+1]+=(2*M_PI*(notes[note]*fvar[osc+1]))/sample_rate;

			lf=(sin(fpoint[osc]));
			rf=(sin(fpoint[osc+1]));

			if (lf > 0 && lf>fcut[osc]){ lf=1;}
			if (lf < 0 && lf<-fcut[osc]){ lf=-1;}

			if (rf > 0 && rf>fcut[osc+1]){ rf=1;}
			if (rf < 0 && rf<-fcut[osc+1]){ rf=-1;}

			o_left=32760*lf;
			o_right=32760*rf;

			left=left+o_left;
			right=right+o_right;
		}
		waveform[along]=(left/number);
		waveform[along+1]=(right/number);
	}	




        fhead[0]=0x46464952;
        fhead[1]=36;
        fhead[2]=0x45564157;
        fhead[3]=0x20746d66;
        fhead[4]=16;
        fhead[5]=65536*chan+1;
        fhead[6]=sample_rate;
        fhead[7]=byte_rate;
        fhead[8]=ba;
        fhead[9]=0x61746164;
        fhead[10]=(size*chan*bits_pers)/8;


    	FILE *record;
        record=fopen("record1000.wav","wb");
        fwrite(fhead,sizeof(int),11,record);
        fwrite(waveform,sizeof(short),size,record);
        fclose (record);
}

/*
void main_osc (long from, long to, short *ring)
{
	long a;
	long vpr[ROWS];
	for (a=from;a<to;a+=2)
	{
		int s;
		long l,r;
		l=0;r=0;

		vp+=2;
                if (vp>=VBUF){vp=0;}


		for (s=0;s<ROWS;s++)
		{
			long p;
			float f,qu,qi,dl,dr,along,blong,gdl,gdr,bdl,bdr,bal,bar,vwalong,vcalong,ghalong,bhalong,lfalong,hfalong,wfalong;
			long lm,rm,sil,sir;
			lm=0;rm=0;
			p=point[s];
			if (p>0)
			{
				float fphase;
				int amp;
				along=(float)p/(float)ssize[s];
				blong=1-along;

				fphase=(along*(float)phase[s]);
				f=notes[tnote[s]];
				amp=envelope[s][(p*(X_ENV-1))/ssize[s]];
				//printf ("%d %ld %d\n",amp,p,ssize[s]);

				if (vvowi[s]==0){ vwalong=1;}
				if (vvowi[s]==1){ vwalong=along;}
				if (vvowi[s]==2){ vwalong=blong;}
				if (vvowi[s]==3){ vwalong=(1+sin((float)(vvowf[s]*p)/441000)/2);}

				if (vvoci[s]==0){ vcalong=1;}
				if (vvoci[s]==1){ vcalong=along;}
				if (vvoci[s]==2){ vcalong=blong;}
				if (vvoci[s]==3){ vcalong=(1+sin((float)(vvocf[s]*p)/441000)/2);}

				if (badhi[s]==0){ bhalong=1;}
				if (badhi[s]==1){ bhalong=along;}
				if (badhi[s]==2){ bhalong=blong;}
				if (badhi[s]==3){ bhalong=(1+sin((float)(badhf[s]*p)/441000)/2);}

				if (goodhi[s]==0){ ghalong=1;}
				if (goodhi[s]==1){ ghalong=along;}
				if (goodhi[s]==2){ ghalong=blong;}
				if (goodhi[s]==3){ ghalong=(1+sin((float)(goodhf[s]*p)/441000)/2);}

				if (lfilti[s]==0){ lfalong=1;}
				if (lfilti[s]==1){ lfalong=along;}
				if (lfilti[s]==2){ lfalong=blong;}
				if (lfilti[s]==3){ lfalong=(1+sin((float)(lfiltf[s]*p)/441000)/2);}

				if (hfilti[s]==0){ hfalong=1;}
				if (hfilti[s]==1){ hfalong=along;}
				if (hfilti[s]==2){ hfalong=blong;}
				if (hfilti[s]==3){ hfalong=(1+sin((float)(hfiltf[s]*p)/441000)/2);}

				if (wobi[s]==0){ wfalong=1;}
				if (wobi[s]==1){ wfalong=along;}
				if (wobi[s]==2){ wfalong=blong;}
				if (wobi[s]==3){ wfalong=(1+sin((float)(wobf[s]*p)/441000)/2);}

				qu=(vcalong*(float)vvoc[s])/300;
				qi=(vwalong*(float)vvow[s])/300;

				basep[s]+=(f+(wob[s]*wfalong))/22050;
				goodp[s]+=((3*f)+(wob[s]*wfalong))/22050;
				badp[s]+=((1.68*f)+(wob[s]*wfalong))/22050;

				bal=sin(fphase+(M_PI*basep[s])); bar=sin((M_PI*basep[s])-fphase);
				gdl=sin(fphase+(M_PI*goodp[s])); gdr=sin((M_PI*goodp[s])-fphase);
				bdl=sin(fphase+(M_PI*badp[s])); bdr=sin((M_PI*badp[s])-fphase);


				if (bal<qi && bal>0){bal=0;} if (-bal<qi && bal<0){bal=0;} if (bal>qu){bal=1;} if (-bal>qu){bal=-1;}
				if (bar<qi && bar>0){bar=0;} if (-bar<qi && bar<0){bal=0;} if (bar>qu){bar=1;} if (-bar>qu){bar=-1;}

				if (gdl<qi && gdl>0){gdl=0;} if (-gdl<qi && gdl<0){gdl=0;} if (gdl>qu){gdl=1;} if (-gdl>qu){gdl=-1;}
				if (gdr<qi && gdr>0){gdr=0;} if (-gdr<qi && gdr<0){bal=0;} if (gdr>qu){gdr=1;} if (-gdr>qu){gdr=-1;}

				if (bdl<qi && bdl>0){bdl=0;} if (-bdl<qi && bdl<0){bal=0;} if (bdl>qu){bdl=1;} if (-bdl>qu){bdl=-1;}
				if (bdr<qi && bdr>0){bdr=0;} if (-bdr<qi && bdr<0){bdr=0;} if (bdr>qu){bdr=1;} if (-bdr>qu){bdr=-1;}


				sil=(300*bal)+(gdl*ghalong*(float)goodh[s])+(bdl*bhalong*(float)badh[s]);
				sir=(300*bar)+(gdr*ghalong*(float)goodh[s])+(bdr*bhalong*(float)badh[s]);


				//filter low pass
				dl=sil-prevl[s];
				dr=sir-prevr[s];
				sil=sil-((lfalong*lfilt[s]*dl)/300); 
				sir=sir-((lfalong*lfilt[s]*dr)/300); 

				//filter high pass
				
				dl=sil-prevl[s];
				dr=sir-prevr[s];
				sil=prevl[s]+((hfalong*hfilt[s]*dl)/300); 
				sir=prevr[s]+((hfalong*hfilt[s]*dr)/300); 

				prevl[s]=sil;prevr[s]=sir; 
				//amp =512 pan=300 
				lm=(amp*sil*lpan[s]);
				rm=(amp*sir*rpan[s]);

				point[s]+=2;
				if (point[s]>=ssize[s]){ basep[s]=0;goodp[s]=0;badp[s]=0;printf("%ld %d\n",point[s],ssize[s]);point[s]=0; }
			}

                        vpr[s]=vp+vdur[s]; if (vpr[s]>=VBUF){vpr[s]=vpr[s]-VBUF;}

			l=l+lm+verb[s][vp];
			r=r+rm+verb[s][vp+1]; 
			//printf ("vprs %d\n",vpr[s]);
			long lv,rv;
			lv=(vamp[s]*(lm+verb[s][vp]))/300;
			rv=(vamp[s]*(rm+verb[s][vp+1]))/300;
			verb[s][vpr[s]]=((lv*vlpan[s])+(rv*vrpan[s]))/300; 
			verb[s][vpr[s]+1]=((rv*vlpan[s])+(lv*vrpan[s]))/300; 
		}
		// pan is 300 amplitude is 512 times too big for a short. plan also or 4 sounds at once. 512*4*300 
		//
		int lc,rc;
		lc=l/6144;
		rc=r/6144;
		if (lc>32600 || lc < -32600 ){ printf ("left clip \n");}
		if (rc>32600 || rc < -32600 ){ printf ("right clip \n");}
		ring[a]=lc;
		ring[a+1]=rc;
		// the verb pointer.
	}
}
*/
