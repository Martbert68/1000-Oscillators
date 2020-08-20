/* grand.c */

#include <alsa/asoundlib.h>
#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>



/* here are our X variables */
Display *display;
XColor    color[100];
int screen;
Window win;
GC gc;
unsigned long black,white;
#define X_SIZE 800 
#define Y_SIZE 550 

/* here are our X routines declared! */
void init_x();
void close_x();
void redraw();

/* sound */
void *spkr();
long where;


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

short *waveform;

void usage ()
{
	printf("usage: grand\n");
	exit (1);
}

int main(int argc,char *argv[])
{
 
	int *fhead,len,chan,sample_rate,bits_pers,byte_rate,ba,size;
	int number,along,osc,note;
	char stop;

	number=1000;

	double fpoint[1000];
	double fvar[1000];
	double fcut[1000];

        init_x();

        fhead=(int *)malloc(sizeof(int)*11);

        len=300;
        chan=2;
        sample_rate=44100;
        bits_pers=16;
        byte_rate=(sample_rate*chan*bits_pers)/8;
        ba=((chan*bits_pers)/8)+bits_pers*65536;
        size=chan*len*sample_rate;
        waveform=(short *)malloc(sizeof(short)*size);

	// setup our oscillatorsi drift up to +-3HZ 
	for (osc=0;osc<number;osc++){ fvar[osc]=10-((double)(rand()%20000)/1000);}
	for (osc=0;osc<number;osc++){ fcut[osc]=0.9;}
	for (osc=0;osc<number;osc++){ fpoint[osc]=(double)(rand()%31415)/5000;}
	//for (osc=0;osc<number;osc++){ fpoint[osc]=0;}

	for (along=0;along<size;along+=2)
	{
		double left,right;
		left=0;right=0;

		for (osc=0;osc<number;osc+=2)
		{
			double lf,rf,lcut,rcut;

			fpoint[osc]+=(2*M_PI*(notes[36]+fvar[osc]))/sample_rate;
			fpoint[osc+1]+=(2*M_PI*(notes[48]+fvar[osc+1]))/sample_rate;

			lf=(sin(fpoint[osc]));
			rf=(sin(fpoint[osc+1]));

			lcut=fcut[osc];
			rcut=fcut[osc+1];

			if (lf > 0 && lf>lcut){ lf=1;}
			if (lf < 0 && lf<-lcut){ lf=-1;}

			if (rf > 0 && rf>rcut){ rf=1;}
			if (rf < 0 && rf<-rcut){ rf=-1;}

			left=left+lf;
			right=right+rf;
		}
		waveform[along]=(350000*left/number);
		waveform[along+1]=(350000*right/number);
		if (along%88200==0){printf("Completed %d seconds \n",along/88200);}
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

	printf ("waiting \n");
	scanf("%c",&stop);

       pthread_t spkr_id;

       struct timespec tim, tim2;
               tim.tv_sec = 0;
        tim.tv_nsec = 100L;


	where=0;
        pthread_create(&spkr_id, NULL, spkr, NULL);
	long my_point,chunk;
	chunk=17640;
	my_point=0;
	while (where<size)
	{
		while (where<my_point+chunk)
		{
                       nanosleep(&tim , &tim2);
		}
		long slice;
		int ex,why;
		ex=0;
		XClearWindow(display, win);
		XSetForeground(display,gc,color[1].pixel);
		for (slice=my_point;slice<my_point+chunk;slice+=4)
		{
			why=(waveform[slice]+32768)/220;
			XDrawPoint(display, win, gc, ex, why);
			if (slice%16==0){ ex++;}
		}
		XSetForeground(display,gc,color[3].pixel);
		ex=0;
		for (slice=my_point+1;slice<my_point+chunk;slice+=4)
		{
			why=(waveform[slice]+32768)/220;
			XDrawPoint(display, win, gc, ex, why+250);
			if (slice%16==1){ ex++;}
		}
		my_point+=chunk;
		XFlush(display);
	}
}


void *spkr(void ) {
        // This handles the speakers.


  int rc;
  int size;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;
  //char *buffer;

  /* Open PCM device for playback. */
  rc = snd_pcm_open(&handle, "default",
                    SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
    fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
    exit(1);
  }
  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);
  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);
  /* Set the desired hardware parameters. */
  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle, params, 2);
  /* 44100 bits/second sampling rate (CD quality) */
  val = 44100;
  snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
  /* Set period size to 32 frames. */
  frames = 64;
  snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
  /* Write the parameters to the aux */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames, &dir);
  //size = frames * 4; /* 2 bytes/sample, 2 channels 
  // size as in number of data points along
  size = frames * 2;

  snd_pcm_hw_params_get_period_time(params, &val, &dir);

  while (1 > 0) {
    rc = snd_pcm_writei(handle, (waveform+where), frames);
    if (rc == -EPIPE) {
      /* EPIPE means underrun */
      fprintf(stderr, "underrun occurred\n");
      snd_pcm_prepare(handle);
    } else if (rc < 0) {
      fprintf(stderr,
              "error from writei: %s\n",
              snd_strerror(rc));
    }  else if (rc != (int)frames) {
      fprintf(stderr,
              "short write, write %d frames\n", rc);
    }
    where+=size;
  }

  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  //free(buffer);

  return 0;
}

void init_x()
{
/* get the colors black and white (see section for details) */
        XInitThreads();
        //x_buffer=(unsigned char *)malloc(sizeof(unsigned char)*4*VX_SIZE*VY_SIZE);
        display=XOpenDisplay((char *)0);
        screen=DefaultScreen(display);
        black=BlackPixel(display,screen),
        white=WhitePixel(display,screen);
        win=XCreateSimpleWindow(display,DefaultRootWindow(display),0,0, X_SIZE, Y_SIZE, 5, white,black);
        XSetStandardProperties(display,win,"PC scope","PC scope",None,NULL,0,NULL);
        XSelectInput(display, win, ExposureMask|ButtonPressMask|KeyPressMask|ButtonReleaseMask|ButtonMotionMask);
        //XSelectInput(display, vwin, ExposureMask|ButtonPressMask|KeyPressMask|ButtonReleaseMask|ButtonMotionMask);
        gc=XCreateGC(display, win, 0,0);
        XSetBackground(display,gc,black); XSetForeground(display,gc,white);
        XClearWindow(display, win); 
        XMapRaised(display, win);
        XMoveWindow(display, win,0,0);
        Visual *visual=DefaultVisual(display, 0);
        Colormap cmap;
        cmap = DefaultColormap(display, screen);
        color[0].red = 65535; color[0].green = 65535; color[0].blue = 65535;
        color[1].red = 65535; color[1].green = 65535; color[1].blue = 0;
        color[2].red = 0; color[2].green = 65535; color[2].blue = 65535;
        color[3].red = 0; color[3].green = 65535; color[3].blue = 65535;
        color[4].red = 0; color[4].green = 65535; color[4].blue = 65535;
        color[5].red = 65535; color[5].green = 65535; color[5].blue = 0;
        XAllocColor(display, cmap, &color[0]);
        XAllocColor(display, cmap, &color[1]);
        XAllocColor(display, cmap, &color[2]);
        XAllocColor(display, cmap, &color[3]);
        XAllocColor(display, cmap, &color[4]);
        XAllocColor(display, cmap, &color[5]);
}

