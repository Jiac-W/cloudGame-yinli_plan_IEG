//  修改自qinyi_style   FFMPEG音视频同步-读取摄像头并编码封装保存
// 	https://blog.csdn.net/quange_style/article/details/90082391

#include "../include/ScreenRecorder.h"
// #include <pthread.h>

// #define STREAM_DURATION   10.0  //视频预设总时长 
#define STREAM_FRAME_RATE 25 /* 25 images/s */  
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P /* default pix_fmt */  

using std::cout;
using std::endl;
using std::string;

// int pix_width = 1920;
// int pix_height = 1080;
int pix_width = 1024;
int pix_height = 768;
bool pipe_flag = true;
int video_frame_count = 0;

// #undef av_err2str
// #define av_err2str(errnum) av_make_error_string((char*)__builtin_alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)

typedef struct OutputStream {  
    AVStream *st;  
    AVCodecContext *enc;
    int64_t next_pts;  
    int samples_count;  
    AVFrame *frame;  
    AVFrame *tmp_frame;  
    float t, tincr, tincr2;  
    struct SwsContext *sws_ctx;  
    struct SwrContext *swr_ctx;  
	
	// modify
	AVStream *in_stream;
} OutputStream;  //相较于标准的结构体少了后面的部分，只有前面的几个参数
  
typedef struct IntputDev {  
	AVCodecContext  *pCodecCtx;  		//pCodecCtx=v_ifmtCtx->streams[videoindex]->codec;  
	AVCodec         *pCodec;  			//pCodec=avcodec_find_decoder(pCodecCtx->codec_id);  
	AVFormatContext *v_ifmtCtx;  		//avformat_alloc_context +  avformat_open_input(&v_ifmtCtx,"/dev/video0",ifmt,NULL)
	int  videoindex;  
	struct SwsContext *img_convert_ctx;  
	AVPacket *in_packet;  		        //(AVPacket *)av_malloc(sizeof(AVPacket)); -->av_read_frame得到内容
	AVFrame *pFrame,*pFrameYUV;         //av_frame_alloc---->解码得到pFrame→→格式转换→→pFrameYUV	avpicture_fill((AVPicture *)pFrameYUV, out_buffer..) 
}IntputDev;  
  
static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)  //log日志相关 打印一些信息 	Definition at line 70 of file muxing.c.
{  
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;  
  
	// printf("pts:%s pts_time:%s duration:%s\n",av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),av_ts2str(pkt->duration));
	/*
    printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",  
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),  
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),  
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),  
           pkt->stream_index);  //这里pts和dts都源自编码前frame.pts 而duration没有被赋值，是初始化时的0
	*/
}  

// pkt写入之前要先处理pkt->pts和pkt->stream_index
static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)  //Definition at line 81 of file muxing.c.
{  
    /* rescale output packet timestamp values from codec(帧序号) to stream timebase */  
    av_packet_rescale_ts(pkt, *time_base, st->time_base);  
			//例如：av_packet_rescale_ts将pkt->pts从帧序号33→时间戳118800
    pkt->stream_index = st->index;  
  
    /* Write the compressed frame to the media file. */  
    log_packet(fmt_ctx, pkt);  
    return av_interleaved_write_frame(fmt_ctx, pkt);  
}  

// 1.*codec = avcodec_find_encoder(codec_id);   2.ost->st = avformat_new_stream(oc, NULL); 		
// 3.ost->enc = c = avcodec_alloc_context3(*codec); 4.填充AVCodecContext *c;  
static void add_stream(OutputStream *ost, AVFormatContext *oc,  AVCodec **codec,  enum AVCodecID codec_id) 
{  
    AVCodecContext *c;  
    int i;  
  
    *codec = avcodec_find_encoder(codec_id);  
    if (!(*codec)) {  fprintf(stderr, "Could not find encoder for '%s'\n",  avcodec_get_name(codec_id));  exit(1);  }  
  
    ost->st = avformat_new_stream(oc, NULL);  
    if (!ost->st) {  fprintf(stderr, "Could not allocate stream\n");  exit(1);  }  
	
    ost->st->id = oc->nb_streams-1;  
    c = avcodec_alloc_context3(*codec);  
    if (!c) {  fprintf(stderr, "Could not alloc an encoding context\n");  exit(1);  }  
	ost->enc = c;  
  
	switch ((*codec)->type) {  
	case AVMEDIA_TYPE_AUDIO:  
		c->sample_fmt  = (*codec)->sample_fmts ?  
			(*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;  
		c->bit_rate    = 64000;  
		c->sample_rate = 44100;  
		if ((*codec)->supported_samplerates) {  
			c->sample_rate = (*codec)->supported_samplerates[0];  
			for (i = 0; (*codec)->supported_samplerates[i]; i++) {  
				if ((*codec)->supported_samplerates[i] == 44100)  
					c->sample_rate = 44100;  
			}  
		}  
		c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);  
		c->channel_layout = AV_CH_LAYOUT_STEREO;  
		if ((*codec)->channel_layouts) {  
			c->channel_layout = (*codec)->channel_layouts[0];  
			for (i = 0; (*codec)->channel_layouts[i]; i++) {  
				if ((*codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)  
					c->channel_layout = AV_CH_LAYOUT_STEREO;  
			}  
		}  
		c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);  
		ost->st->time_base = (AVRational){ 1, c->sample_rate };  
		break;  
  
	case AVMEDIA_TYPE_VIDEO:  
		c->codec_id = codec_id;  
  
		c->bit_rate = 800000;  
		/* Resolution must be a multiple of two. */  
		c->width    = pix_width;  
		c->height   = pix_height;  
		/* timebase: This is the fundamental unit of time (in seconds) in terms 
		 * of which frame timestamps are represented. For fixed-fps content, 
		 * timebase should be 1/framerate and timestamp increments should be 
		 * identical to 1. */  
		ost->st->time_base = (AVRational){ 1, STREAM_FRAME_RATE };  
		c->time_base       = ost->st->time_base;  
  
		c->gop_size      = 12; /* 12 emit one intra frame every twelve frames at most */  
		c->pix_fmt       = STREAM_PIX_FMT;  
		if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {  
			/* just for testing, we also add B-frames */  
			c->max_b_frames = 2;  
		}  
		if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {  
			/* Needed to avoid using macroblocks in which some coeffs overflow. 
			 * This does not happen with normal video, it just happens here as 
			 * the motion of the chroma plane does not match the luma plane. */  
			c->mb_decision = 2;  
		}  
	break;  
  
	default:  
		break;  
	}  
  
	/* Some formats want stream headers to be separate. */  
	if (oc->oformat->flags & AVFMT_GLOBALHEADER)  
		c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;  //oc的格式需要分离的文件头→→c的格式也需要分离的文件头
}  

/**************************************************************************/  
/* video output */  
static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)  //申请一个指定长宽像素的AVFrame
{  
	AVFrame *picture;  
	int ret;  
  
	picture = av_frame_alloc();  //this only allocates the AVFrame itself, not the data buffers.
	//Those must be allocated through other means, e.g. with av_frame_get_buffer() or manually.
	if (!picture)  
		return NULL;  
  
	picture->format = pix_fmt;  
	picture->width  = width;  
	picture->height = height;  
  
	/* allocate the buffers for the frame data */  
	ret = av_frame_get_buffer(picture, 32);  //为音频或视频数据分配新的缓冲区。
	if (ret < 0) {  
		fprintf(stderr, "Could not allocate frame data.\n");  
		exit(1);  
	}  
  
	return picture;  
}  

// 1.avcodec_open2(c, codec, &opt);    2.allocate and init a re-usable frame 3.prepar ost->tmp_frame 4.avcodec_parameters_from_context
static void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)  
{  
	int ret;  
	AVCodecContext *c = ost->enc;  
	AVDictionary *opt = NULL;  
  
	av_dict_copy(&opt, opt_arg, 0);  
  
	// modify
	// ultrafast、superfast、veryfast、faster、fast、medium
	av_opt_set(c->priv_data, "preset", "ultrafast", 0);
	av_opt_set(c->priv_data, "tune", "zerolatency", 0);
	// av_opt_set(c->priv_data, "profile", "high", 0);
	// av_opt_set(c->priv_data, "sc_threshold", "499", 0);
	// modify

	/* open the codec */  
	ret = avcodec_open2(c, codec, &opt);  
	if (ret < 0) {  
		// fprintf(stderr, "Could not open video codec: %s\n", av_err2str(ret));  
		cout<< "Could not open video codec.\n";
		exit(1);  
	}  
	av_dict_free(&opt);  
  
	/* allocate and init a re-usable frame */  
	ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);  
	if (!ost->frame) {  fprintf(stderr, "Could not allocate video frame\n");  exit(1);  }  
	//printf("ost->frame alloc success fmt=%d w=%d h=%d\n",c->pix_fmt,c->width, c->height);  
  
  
	/* If the output format is not YUV420P, then a temporary YUV420P 
	 * picture is needed too. It is then converted to the required 
	 * output format. */  
	ost->tmp_frame = NULL;  
	if (c->pix_fmt != AV_PIX_FMT_YUV420P) {  
		ost->tmp_frame = alloc_picture(AV_PIX_FMT_YUV420P, c->width, c->height);  
		if (!ost->tmp_frame) {  
			fprintf(stderr, "Could not allocate temporary picture\n");  
			exit(1);  
		}  
	}  
  
	/* copy the stream parameters to the muxer */  
	ret = avcodec_parameters_from_context(ost->st->codecpar, c);  
	if (ret < 0) {  
		fprintf(stderr, "Could not copy the stream parameters\n");  
		exit(1);  
	}  
}  
  
// encode one video frame and send it to the muxer  return 1 when encoding is finished, 0 otherwise 
// 1.avcodec_encode_video2 	2.调用write_frame函数
static int write_video_frame1(AVFormatContext *oc, OutputStream *ost, AVFrame *frame, int64_t start_time)  
{  
	int ret;  
	AVCodecContext *c;  
	int got_packet = 0;  
	AVPacket pkt = { 0 };  
  
	if(frame==NULL)  return 1;  
  
	c = ost->enc;  
	av_init_packet(&pkt);  
  
	/* encode the image */  
	ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);  //pkt.pts搬自frame.pts
	if (ret < 0) 
    {  
		cout<< "Error encoding video frame.\n" <<endl;
        exit(1);  
	}  
  
	// printf("---video- pkt.pts=%s     ",av_ts2str(pkt.pts));  //av_ts2str即将包含时间戳的int64_t变成char*buf    起始就是输出帧序号
	if (pkt.pts == 0) 
		printf("----st.num=%d st.den=%d codec.num=%d codec.den=%d---------\n",ost->st->time_base.num,ost->st->time_base.den,  c->time_base.num,c->time_base.den);  
	// 输出流的AVRational 和 编码器的AVRational
	
	if (got_packet) 
	{  
		// // Modify
    	// // 延时方案2: 根据pts时间与系统时间的关系来计算延时时间，   该方案更优
        // AVRational  dst_time_base = {1, AV_TIME_BASE};
        // int64_t pts_time = av_rescale_q(pkt.pts, ost->in_stream->time_base, dst_time_base); 
        // int64_t now_time = av_gettime() - start_time;
        // if( pts_time > now_time)
        //     av_usleep(pts_time - now_time);
		// cout<< "TIMEs= " << pts_time - now_time <<endl;
		// // Modify
        printf("2. Writed %5d video frame %5d.  ", ++video_frame_count, pkt.size);
		ret = write_frame(oc, &c->time_base, ost->st, &pkt); 
	}
	else 
	{  
		cout<< "This is !got_packet.\n";			
		ret = 0;
	}  
  
	if (ret < 0) {  
        // fprintf(stderr, "Error while writing video frame: %s\n", av_err2str(ret));  
		cout<< "Error while writing video frame.\n" <<endl;		
        exit(1);  
	}  
	
	return (frame || got_packet) ? 0 : 1;  
}  

//1.av_frame_make_writable		2.av_read_frame--avcodec_decode_video2---sws_scale
//数据流：input->in_packet----input->pFrame----ost->frame
static AVFrame *get_video_frame1(OutputStream *ost,IntputDev* input,int *got_pic)
{  
	int ret, got_picture;  
	AVCodecContext *c = ost->enc;  
	AVFrame * ret_frame = NULL;  
  
	/* when we pass a frame to the encoder, it may keep a reference to it 
	 * internally; make sure we do not overwrite it here */  
	if (av_frame_make_writable(ost->frame) < 0)  
		exit(1);  
	  
	if(av_read_frame(input->v_ifmtCtx, input->in_packet)>=0)
	{  
		// Modify
		ost->in_stream = input->v_ifmtCtx->streams[input->in_packet->stream_index];
		// Modify

		if(input->in_packet->stream_index == input->videoindex)
		{  
			ret = avcodec_decode_video2(input->pCodecCtx, input->pFrame, &got_picture, input->in_packet);  
			*got_pic = got_picture;  
  
			if(ret < 0){  
				printf("Decode Error.\n");  
				// av_free_packet(input->in_packet);  
				av_packet_unref(input->in_packet);				
				return NULL;  
			}  
			if(got_picture){  
				sws_scale(input->img_convert_ctx, (const unsigned char* const*)input->pFrame->data, input->pFrame->linesize, 0, input->pCodecCtx->height, ost->frame->data,  ost->frame->linesize);  
				ost->frame->pts = ost->next_pts++;  
				ret_frame= ost->frame;  
		        // printf("\n1-Got a frame %5d.\n", input->in_packet->size);
			}  
		}  
		// av_free_packet(input->in_packet);  
		av_packet_unref(input->in_packet);
	}  
	return ret_frame;  
}  

static void close_stream(AVFormatContext *oc, OutputStream *ost)  
{  
	avcodec_free_context(&ost->enc);  
	av_frame_free(&ost->frame);  
	av_frame_free(&ost->tmp_frame);  
	sws_freeContext(ost->sws_ctx);  
	swr_free(&ost->swr_ctx);  
}  

// for named pipe
void* another(void* arg)
{
    int fd;
    char * myfifo = "./myfifo";
    // mkfifo(<pathname>, <permission>)
    mkfifo(myfifo, 0666);
    int write_pipe[2] = {0};

	while(1)
	{
		fd = open(myfifo, O_RDONLY);		
		read(fd, write_pipe, sizeof(write_pipe));
		pipe_flag = write_pipe[0];
	}
	close(fd);
}	

class timeWatch
{
 public:
  void start() noexcept{
	  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &this->start_ts);
	  this->is_started = true;
  }

  long get_ns()  const noexcept{
	  struct timespec end_ts;
	  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_ts);
	  long dur_s = end_ts.tv_sec - start_ts.tv_sec;
	  long dur_ns = dur_s * 1000000000L + end_ts.tv_nsec - start_ts.tv_nsec;
      return dur_ns;
  }

  double get_us() const noexcept{  //微秒
      return get_ns()/1000.0;
  }
  double get_ms() const noexcept{  //毫秒    
      return get_ns()/1000000.0;
  }  
 private:
    struct timespec start_ts;
    bool is_started;
};

/**************************************************************/  
  
int main(int argc, char **argv)  
{  	
	int ret;  
	int have_video = 0;  
	int encode_video = 0;   
  	// int pix_width = 1920;
	// int pix_height = 1080;
	IntputDev video_input = { 0 };  // 总输入
	OutputStream video_st = { 0 };  // 输出,即ost
	
	// av_register_all();	
	avcodec_register_all();
	avdevice_register_all();
	
	AVDictionary *options = NULL;	
	AVCodecContext  *pCodecCtx;  
	AVCodec         *pCodec = NULL;  
	AVFormatContext *v_ifmtCtx = avformat_alloc_context();  

	av_dict_set(&options, "video_size", "1024*768", AV_DICT_MATCH_CASE);
	av_dict_set(&options, "framerate", "30", AV_DICT_MATCH_CASE);
	// av_dict_set(&options, "draw_mouse", "1", AV_DICT_MATCH_CASE);
	
	
	AVInputFormat *ifmt = av_find_input_format("x11grab");  //screen

	if (ifmt == NULL)
    {
        cout << "\nav_find_input_format not found......" << endl;
		exit(1);
    }

	if(avformat_open_input(&v_ifmtCtx,":0.0+0,0",ifmt,&options) != 0){
		printf("Couldn't open input stream./dev/video1\n");
		return -1;
	}  
    av_dump_format(v_ifmtCtx, 0, ":0.0+0,0", 0);

	if(avformat_find_stream_info(v_ifmtCtx,NULL)<0){
		printf("Couldn't find stream information.\n");
		return -1;
	}  

	// find video stream index
	int videoindex=-1;  
	for(int i=0; i<v_ifmtCtx->nb_streams; i++)
	{
		if(v_ifmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			videoindex=i;
			// modify
			AVStream *stream_in = v_ifmtCtx->streams[videoindex];
			pCodec = avcodec_find_decoder(stream_in->codecpar->codec_id);
			if(pCodec == NULL){
				printf("Codec not found.\n");  
				return -1;  
			}			
			pCodecCtx = avcodec_alloc_context3(pCodec);
			ret = avcodec_parameters_to_context(pCodecCtx, stream_in->codecpar);
			if (ret < 0) {
				printf("Failed to copy context input to output stream codec context\n");
				return -1;
			}
			// modify
			break;
		}  
		if(i == v_ifmtCtx->nb_streams-1){
			printf("Couldn't find a video stream.\n");
			return -1;
		}
	}		

	// pCodecCtx = v_ifmtCtx->streams[videoindex]->codec;  // old version

	printf("pCodecCtx->width=%d pCodecCtx->height=%d \n",pCodecCtx->width, pCodecCtx->height); 
	
	// if ((pCodec = avcodec_find_decoder(pCodecCtx->codec_id)) == NULL){
	// 	printf("Codec not found.\n");  
	// 	return -1;  
	// }
	
	if(avcodec_open2(pCodecCtx, pCodec, NULL)<0){
		printf("Could not open codec.\n");
		return -1;
	}  
  
	cout<< "Step.1 Begining Screen Capture.\n";
	AVFrame *pFrame 	= av_frame_alloc();  
	AVFrame *pFrameYUV 	= av_frame_alloc();   

	// modify
	int nbytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,pCodecCtx->width,pCodecCtx->height,32);	
	// modify

	unsigned char *out_buffer = (unsigned char *)av_malloc(nbytes);  

	// avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);  
	int value = av_image_fill_arrays( pFrameYUV->data, pFrameYUV->linesize, out_buffer , 
								  AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height,1 ); 	

	struct SwsContext *img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);   
	AVPacket *in_packet = NULL;
	if ((in_packet = (AVPacket *)av_malloc(sizeof(AVPacket))) == NULL){
		printf("error while av_malloc");
		return -1;
	}  

	// 关联IntputDev video_input和其子成员
	video_input.img_convert_ctx = img_convert_ctx;  
	video_input.in_packet = in_packet;  
	video_input.pCodecCtx = pCodecCtx;  
	video_input.pCodec = pCodec;  
	video_input.v_ifmtCtx = v_ifmtCtx;  	//AVFormatContext *
	video_input.videoindex = videoindex;  
	video_input.pFrame = pFrame;  		//packet解码得到
	video_input.pFrameYUV = pFrameYUV;  	//pFrame->sws->pFrameYUV
	
	// //***********************输出
	// OutputStream video_st = { 0 }; //即ost
	// const char *filename = "/home/wu/tencent/video_capture/devel/lib/capture_codec/flv_output.flv";
	// const char *filename = "rtmp://localhost:1935/live/home";//输出 URL（Output URL）[RTMP]	
	// const char *filename = "rtmp://localhost:1935/rtmp_live/wjc";//输出 URL（Output URL）[RTMP]			
	// const char *filename = "flv_output.mp4";//输出 URL（Output URL）[RTMP]		
	
	// for rtmp
	// const char *filename = "rtmp://121.5.5.221:1935/rtmp_live/wjc";//输出 URL（Output URL）[RTMP]	

	// for udp
	// const char *filename = "rtsp://localhost:8554/wjc";//输出 URL（Output URL）[rtsp]	
	const char *filename = "rtsp://127.0.0.1:8554/wjc";//输出 URL（Output URL）[rtsp]	

	AVFormatContext *oc;  
	AVCodec *video_codec;

	// avformat_alloc_output_context2(&oc, NULL, NULL, filename); 
	// avformat_alloc_output_context2(&oc, NULL, "mpegts", filename); // UDP	
	// avformat_alloc_output_context2(&oc, NULL, "flv", filename); 	  // RTMP
	avformat_alloc_output_context2(&oc, NULL, "rtsp", filename); // UDP	

	if (!oc) {
		printf("Could not deduce output format from file extension: using MPEG.\n");
		avformat_alloc_output_context2(&oc, NULL, "mpeg", filename);  
		return 1;
	}  
	
	AVOutputFormat *fmt = oc->oformat;  // AVFormatContext-> AVOutputFormat *oformat
	/* Add the audio and video streams using the default format codecs and initialize the codecs. */  
	if (fmt->video_codec != AV_CODEC_ID_NONE) {  
		// OutputStream *ost, AVFormatContext *oc,  AVCodec **codec,  enum AVCodecID codec_id		
		// add_stream(&video_st, oc, &video_codec, fmt->video_codec);  
		add_stream(&video_st, oc, &video_codec, AV_CODEC_ID_H264);  
		/* Add an output stream(ost->st) && add an AVCodecContext(即ost->enc)并填充 */
		// 1.*codec = avcodec_find_encoder(codec_id);   2.ost->st = avformat_new_stream(oc, NULL); 		
		// 3.ost->enc = c = avcodec_alloc_context3(*codec); 4.填充AVCodecContext *c;  			
		have_video = 1;  
		encode_video = 1;  
	}  

	/* Now that all the parameters are set, we can open the audio and  video codecs and allocate the necessary encode buffers. */  
	// 1.avcodec_open2(c, codec, &opt); 2.allocate and init a re-usable frame; 3.prepar ost->tmp_frame; 4.avcodec_parameters_from_context	
	if (have_video)  open_video(oc, video_codec, &video_st, NULL);  
	av_dump_format(oc, 0, filename, 1);  

	/* open the output file, if needed creat AVIOcontext*/  
	if (!(fmt->flags & AVFMT_NOFILE)) {  
		if (avio_open(&oc->pb, filename, AVIO_FLAG_WRITE) < 0){
			cout<< "Could not open output files.\n";
			return -1;
		}  
	}  

	// Modify
	int64_t start_time = av_gettime();
	AVDictionary *format_opts = NULL;
	// av_dict_set(&format_opts, "stimeout", std::to_string(2 * 1000000).c_str(), 0);
	// av_dict_set(&format_opts, "rtsp_transport", "udp", 0);	
	av_dict_set(&format_opts, "rtsp_transport", "tcp", 0);		
	// Modify

	/* Write the stream header, if any. */  
	if (avformat_write_header(oc, &format_opts) < 0){
		cout<< "Error occurred when opening output file.\n";
		return 1;
	}  
  
	cout<< "Step.2 encoding video.\n";	

	int got_pic = 0;  

    pthread_t id;
    pthread_create( &id, NULL, another, &pipe_flag);  //开子线程

	// for time cal
	timeWatch time_cal;
	bool oneshot = true;

	while (1)
	{  		
		// std::cout<< "\n1. 开始推流，Status= " << pipe_flag << std::endl;
		if(pipe_flag)
		{
			time_cal.start();

			// frame.pts源自pkt.pts 又源自0不停地+1 即帧序号
			// 0.av_compare_tsc超时结束		1.av_frame_make_writable		
			// 2.av_read_frame--avcodec_decode_video2---sws_scale->->->得到解码后的->frame
			AVFrame *frame = get_video_frame1(&video_st,&video_input,&got_pic);  
			if(!got_pic)  
			{  
				cout<< "!got_pic.\n";
				usleep(10000);  //单位是微秒（百万分之一秒）。
				continue;
			}  
			encode_video = !write_video_frame1(oc, &video_st, frame, start_time);

			double ms = time_cal.get_ms();
			std::cout<< "编码耗时 " << ms << "ms" <<std::endl;
			// oneshot = true;
		}
		else if(!pipe_flag)
		{
			std::cout<< pipe_flag << ". 推流已暂停。\n" <<std::endl;
			// oneshot = false;
		}
	}  
	av_write_trailer(oc);  
	sws_freeContext(video_input.img_convert_ctx);  
	avcodec_close(video_input.pCodecCtx);  
	av_free(video_input.pFrameYUV);  
	av_free(video_input.pFrame);      
	avformat_close_input(&video_input.v_ifmtCtx);  
	/* Close each codec. */  
	if (have_video)  close_stream(oc, &video_st);  
  	/* Close the output file. */  
	if (!(fmt->flags & AVFMT_NOFILE))  avio_closep(&oc->pb);  
	/* free the stream */  
	avformat_free_context(oc);  

    pthread_join( id, NULL );

	return 0;  
}  
