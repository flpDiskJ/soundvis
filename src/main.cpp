// Code by Jake Aigner December, 2023

#include <SDL.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <ctime>
#include <math.h>

const int MAX_RECORDING_DEVICES = 10;

//Maximum recording time
const int SAMPLE_SIZE = 512;
const int SAMPLES_PER_FRAME = 1;
const int FRAME_DATA_SIZE = SAMPLE_SIZE * SAMPLES_PER_FRAME;
const int BUFFER_LEN = FRAME_DATA_SIZE * 2;
const int sFreq = 44100;

// NOTE: screen width must be equal to or less than SAMPLE_SIZE * SAMPLES_PER_FRAME!

const int screen_w = 640;
const int screen_h = 480;
const int center = screen_h / 2;
const double advance = (double)FRAME_DATA_SIZE / (double)screen_w;
const double amp = (double)screen_h / 255.0;

int red_val = 255;
int gre_val = 255;
int blu_val = 0;
int mode = 1;

void color_yellow()
{
    red_val = 255;
    gre_val = 255;
    blu_val = 0;
}

void color_red()
{
    red_val = 255;
    gre_val = 0;
    blu_val = 0;
}

void color_green()
{
    red_val = 0;
    gre_val = 255;
    blu_val = 0;
}

void color_blue()
{
    red_val = 0;
    gre_val = 0;
    blu_val = 255;
}

void color_orange()
{
    red_val = 255;
    gre_val = 100;
    blu_val = 0;
}

//Recieved audio spec
SDL_AudioSpec gReceivedRecordingSpec;
SDL_AudioSpec gReceivedPlaybackSpec;

//Recording data buffer
Uint8* gRecordingBuffer = NULL;

//Size of data buffer
Uint32 gBufferByteSize = 0;

//Position in data buffer
Uint32 gBufferBytePosition = 0;

//Maximum position in data buffer for recording
Uint32 gBufferByteMaxPosition = 0;

int gRecordingDeviceCount = 0;

int sample_count = 0;

void audioRecordingCallback(void* userdata, Uint8* stream, int len )
{
	//Copy audio from stream
	std::memcpy(&gRecordingBuffer[ gBufferBytePosition ], stream, len);

	//Move along buffer
	gBufferBytePosition += len;
    if(gBufferBytePosition > gBufferByteMaxPosition)
    {
        gBufferBytePosition = 0;
        sample_count++;
    }
}

void circle(SDL_PixelFormat *fmt, void *pixels, int xCenter, int yCenter, int radius)
{
    int x, y, r2;
    r2 = radius * radius;
    for (x = -radius; x <= radius; x++) {
        y = (int) (sqrt(r2 - x*x) + 0.5);
        if (y + yCenter < screen_h - 1 && x + xCenter < screen_w - 1)
        {
            ((Uint32*)pixels)[ (xCenter + x) + ( (yCenter + y) * screen_w ) ] = SDL_MapRGB(fmt, blu_val, gre_val, red_val);
            ((Uint32*)pixels)[ (xCenter + x) + ( (yCenter - y) * screen_w ) ] = SDL_MapRGB(fmt, blu_val, gre_val, red_val);
            ((Uint32*)pixels)[ (xCenter + x + 1) + ( (yCenter + y + 1) * screen_w ) ] = SDL_MapRGB(fmt, blu_val, gre_val, red_val);
            ((Uint32*)pixels)[ (xCenter + x + 1) + ( (yCenter - y + 1) * screen_w ) ] = SDL_MapRGB(fmt, blu_val, gre_val, red_val);
        }
    }
}

SDL_Rect meter, meter2;

void scope_gen(SDL_Texture* tex, SDL_Window *win)
{
    int val = 0;
    void *pixels;
    int pitch;
    SDL_PixelFormat *fmt = SDL_AllocFormat(SDL_GetWindowPixelFormat(win));
    SDL_LockTexture(tex, NULL, &pixels, &pitch);
    double pos = 0;
    int actual_pos = 0;
    int avr = 0, av_count = 0;
    int arr_pos = 0;
    int peak = 0;
    for (int x = 0; x < screen_w; x++)
    {
        actual_pos = (int)pos;
        val = gRecordingBuffer[actual_pos];
        val = val * amp;
        pos += advance;
        for (int y = 0; y < screen_h; y++)
        {
            if (mode == 1)
            {
                if (center < val && y >= center && y <= val)
                {
                    ((Uint32*)pixels)[ x + ( y * screen_w ) ] = SDL_MapRGB(fmt, blu_val, gre_val, red_val);
                } else if (center > val && y < center && y >= val)
                {
                    ((Uint32*)pixels)[ x + ( y * screen_w ) ] = SDL_MapRGB(fmt, blu_val, gre_val, red_val);
                } else
                {
                    ((Uint32*)pixels)[ x + ( y * screen_w ) ] = SDL_MapRGB(fmt, 0, 0, 0);
                }
            } else if (mode == 2)
            {
                if (y > val - 30 && y < val + 30)
                {
                    ((Uint32*)pixels)[ x + ( y * screen_w ) ] = SDL_MapRGB(fmt, blu_val, gre_val, red_val);
                } else
                {
                    ((Uint32*)pixels)[ x + ( y * screen_w ) ] = SDL_MapRGB(fmt, 0, 0, 0);
                }
            } else if (mode == 3)
            {
                if (y > val + center -5 || y < (screen_h - val) - center +5)
                {
                    ((Uint32*)pixels)[ x + ( y * screen_w ) ] = SDL_MapRGB(fmt, blu_val, gre_val, red_val);
                } else
                {
                    ((Uint32*)pixels)[ x + ( y * screen_w ) ] = SDL_MapRGB(fmt, 0, 0, 0);
                }
            } else if (mode == 4)
            {
                if (y > val || y < (screen_h - val))
                {
                    ((Uint32*)pixels)[ x + ( y * screen_w ) ] = SDL_MapRGB(fmt, 0, 0, 0);
                } else
                {
                    ((Uint32*)pixels)[ x + ( y * screen_w ) ] = SDL_MapRGB(fmt, blu_val, gre_val, red_val);
                }
            } else {
                ((Uint32*)pixels)[ x + ( y * screen_w ) ] = SDL_MapRGB(fmt, 0, 0, 0);
                avr += val;
                av_count++;
                if (val > peak)
                {
                    peak = val;
                }
            }
        }
    }
    if (mode == 5)
    {
        avr = (avr / av_count);
        circle(fmt, pixels, screen_w / 2, screen_h / 2, (2*avr) - (screen_w/2.4));
        peak = 2 * (peak - center);
        if (peak < 0) {peak = 0 - peak;}
        if (meter.h < peak)
        {
            meter.y = meter2.y = screen_h - peak;
            meter.h = meter2.h = peak;
        }
    }
    SDL_UnlockTexture(tex);
    SDL_FreeFormat(fmt);
}

int main()
{
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return 1;
	}

    SDL_Window* window = NULL;

    SDL_Renderer* render = NULL;

    meter.x = screen_w / 10;
    meter.y = meter2.y = screen_h-1;
    meter.w = meter2.w = screen_w / 10;
    meter.h = meter2.h = 1;
    meter2.x = screen_w - meter.x - meter.w;

	SDL_AudioDeviceID recordingDeviceId = 0;
	SDL_AudioDeviceID playbackDeviceId = 0;

	SDL_AudioSpec desiredRecordingSpec;

	SDL_zero(desiredRecordingSpec);
	desiredRecordingSpec.freq = sFreq;
	desiredRecordingSpec.format = AUDIO_U8;
	desiredRecordingSpec.channels = 1;
	desiredRecordingSpec.samples = SAMPLE_SIZE;
	desiredRecordingSpec.callback = audioRecordingCallback;

	gRecordingDeviceCount = SDL_GetNumAudioDevices(SDL_TRUE);

	if(gRecordingDeviceCount < 1)
	{
		printf( "Unable to get audio capture device! SDL Error: %s\n", SDL_GetError() );
		return 0;
	}

	int index;

    printf("Hit escape key to close program window\n");

	for(int i = 0; i < gRecordingDeviceCount; ++i)
	{
		//Get capture device name
		const char* deviceName = SDL_GetAudioDeviceName(i, SDL_TRUE);

		printf("%d - %s\n", i, deviceName);
	}

	printf("Choose audio\n");
	scanf("%d", &index);

	//Open recording device
	recordingDeviceId = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(index, SDL_TRUE), SDL_TRUE, &desiredRecordingSpec, &gReceivedRecordingSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);

	// Device failed to open
	if(recordingDeviceId == 0)
	{
		//Report error
		printf("Failed to open recording device! SDL Error: %s", SDL_GetError() );
		return 1;
	}

	//Calculate per sample bytes
	int bytesPerSample = gReceivedRecordingSpec.channels * (SDL_AUDIO_BITSIZE(gReceivedRecordingSpec.format) / 8);

	//Calculate bytes per second
	int bytesPerSecond = gReceivedRecordingSpec.freq * bytesPerSample;

	//Calculate buffer size
	gBufferByteSize = BUFFER_LEN * 2;

	//Calculate max buffer use
	gBufferByteMaxPosition = BUFFER_LEN;

	//Allocate and initialize byte buffer
	gRecordingBuffer = new Uint8[gBufferByteSize];
	std::memset(gRecordingBuffer, 0, gBufferByteSize);

    window = SDL_CreateWindow("Sound Visualizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    screen_w, screen_h, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("Window creation failed!\n");
        return 2;
    }

    render = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (render == NULL)
    {
        printf("Failed to create renderer!\n");
        return 2;
    }

    SDL_Texture* scope_tex = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, screen_w, screen_h);
    SDL_Rect scope_rec;
    scope_rec.x = 0;
    scope_rec.y = 0;
    scope_rec.w = screen_w;
    scope_rec.h = screen_h;

	// begin recording
	//Go back to beginning of buffer
	gBufferBytePosition = 0;
	SDL_PauseAudioDevice( recordingDeviceId, SDL_FALSE );
    bool run = true;
    SDL_Event e;
    int drop_delay = 0;

    SDL_ShowCursor(SDL_DISABLE);

    while (run)
    {
        while(SDL_PollEvent(&e))
        {
            switch (e.type)
            {
                case SDL_QUIT:
                    run = false;
                    break;
                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_ESCAPE)
                    {
                        run = false;
                    } else {
                        switch (e.key.keysym.sym)
                        {
                            case SDLK_y:
                                color_yellow();
                                break;
                            case SDLK_b:
                                color_blue();
                                break;
                            case SDLK_g:
                                color_green();
                                break;
                            case SDLK_r:
                                color_red();
                                break;
                            case SDLK_o:
                                color_orange();
                                break;
                            case SDLK_1:
                                mode = 1;
                                break;
                            case SDLK_2:
                                mode = 2;
                                break;
                            case SDLK_3:
                                mode = 3;
                                break;
                            case SDLK_4:
                                mode = 4;
                                break;
                            case SDLK_5:
                                mode = 5;
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        if (sample_count >= SAMPLES_PER_FRAME)
        {
            sample_count = 0;
            scope_gen(scope_tex, window);
            SDL_SetRenderDrawColor(render, 0, 0, 0, 0xFF); // Background color
            SDL_RenderClear(render);
            SDL_RenderCopy(render, scope_tex, NULL, &scope_rec);
            if (mode == 5)
            {
                SDL_SetRenderDrawColor(render, blu_val, red_val, gre_val, 0xFF);
                SDL_RenderFillRect(render, &meter);
                SDL_RenderFillRect(render, &meter2);
            }
            SDL_RenderPresent(render);
        }
        if (meter.h > 1 && mode == 5 && drop_delay == 0)
        {
            drop_delay = 1000;
            meter.h--; meter2.h--;
            meter.y++; meter2.y++;
        } else if (drop_delay > 0)
        {
            drop_delay--;
        }
    }

	if( gRecordingBuffer != NULL )
	{
		delete[] gRecordingBuffer;
		gRecordingBuffer = NULL;
	}

    SDL_CloseAudioDevice(recordingDeviceId);
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
