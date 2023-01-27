INCLUDES = -Iinclude -I../UsageEnvironment/include -I../groupsock/include
PREFIX = /usr/local
LIBDIR = $(PREFIX)/lib
##### Change the following for your environment:
# Comment out the following line to produce Makefiles that generate debuggable code:
NODEBUG=1

# The following definition ensures that we are properly matching
# the WinSock2 library file with the correct header files.
# (will link with "ws2_32.lib" and include "winsock2.h" & "Ws2tcpip.h")
TARGETOS = WINNT

# If for some reason you wish to use WinSock1 instead, uncomment the
# following two definitions.
# (will link with "wsock32.lib" and include "winsock.h")
#TARGETOS = WIN95
#APPVER = 4.0

!include    <ntwin32.mak>

UI_OPTS =		$(guilflags) $(guilibsdll)
# Use the following to get a console (e.g., for debugging):
CONSOLE_UI_OPTS =		$(conlflags) $(conlibsdll)
CPU=i386

TOOLS32	=		C:\Program Files\Microsoft Visual Studio 10.0\VC
COMPILE_OPTS =		$(INCLUDES) $(cdebug) $(cflags) $(cvarsdll) -I. -I"$(TOOLS32)\include"
C =			c
C_COMPILER =		"$(TOOLS32)\bin\cl"
C_FLAGS =		$(COMPILE_OPTS)
CPP =			cpp
CPLUSPLUS_COMPILER =	$(C_COMPILER)
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS)
OBJ =			obj
LINK =			$(link) -out:
LIBRARY_LINK =		lib -out:
LINK_OPTS_0 =		$(linkdebug) msvcirt.lib
LIBRARY_LINK_OPTS =	
LINK_OPTS =		$(LINK_OPTS_0) $(UI_OPTS)
CONSOLE_LINK_OPTS =	$(LINK_OPTS_0) $(CONSOLE_UI_OPTS)
SERVICE_LINK_OPTS =     kernel32.lib advapi32.lib shell32.lib -subsystem:console,$(APPVER)
LIB_SUFFIX =		lib
LIBS_FOR_CONSOLE_APPLICATION =
LIBS_FOR_GUI_APPLICATION =
MULTIMEDIA_LIBS =	winmm.lib
EXE =			.exe
PLATFORM = Windows

rc32 = "$(TOOLS32)\bin\rc"
.rc.res:
	$(rc32) $<
##### End of variables to change

NAME = libliveMedia
LIVEMEDIA_LIB = $(NAME).$(LIB_SUFFIX)
ALL = $(LIVEMEDIA_LIB)
all:	$(ALL)

.$(C).$(OBJ):
	$(C_COMPILER) -c $(C_FLAGS) $<
.$(CPP).$(OBJ):
	$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) $<

MP3_SOURCE_OBJS = MP3FileSource.$(OBJ) MP3Transcoder.$(OBJ) MP3ADU.$(OBJ) MP3ADUdescriptor.$(OBJ) MP3ADUinterleaving.$(OBJ) MP3ADUTranscoder.$(OBJ) MP3StreamState.$(OBJ) MP3Internals.$(OBJ) MP3InternalsHuffman.$(OBJ) MP3InternalsHuffmanTable.$(OBJ) MP3ADURTPSource.$(OBJ)
MPEG_SOURCE_OBJS = MPEG1or2Demux.$(OBJ) MPEG1or2DemuxedElementaryStream.$(OBJ) MPEGVideoStreamFramer.$(OBJ) MPEG1or2VideoStreamFramer.$(OBJ) MPEG1or2VideoStreamDiscreteFramer.$(OBJ) MPEG4VideoStreamFramer.$(OBJ) MPEG4VideoStreamDiscreteFramer.$(OBJ) H264VideoStreamFramer.$(OBJ) H264VideoStreamDiscreteFramer.$(OBJ) MPEGVideoStreamParser.$(OBJ) MPEG1or2AudioStreamFramer.$(OBJ) MPEG1or2AudioRTPSource.$(OBJ) MPEG4LATMAudioRTPSource.$(OBJ) MPEG4ESVideoRTPSource.$(OBJ) MPEG4GenericRTPSource.$(OBJ) $(MP3_SOURCE_OBJS) MPEG1or2VideoRTPSource.$(OBJ) MPEG2TransportStreamMultiplexor.$(OBJ) MPEG2TransportStreamFromPESSource.$(OBJ) MPEG2TransportStreamFromESSource.$(OBJ) MPEG2TransportStreamFramer.$(OBJ) ADTSAudioFileSource.$(OBJ)
H263_SOURCE_OBJS = H263plusVideoRTPSource.$(OBJ) H263plusVideoStreamFramer.$(OBJ) H263plusVideoStreamParser.$(OBJ)
AC3_SOURCE_OBJS = AC3AudioStreamFramer.$(OBJ) AC3AudioRTPSource.$(OBJ)
DV_SOURCE_OBJS = DVVideoStreamFramer.$(OBJ) DVVideoRTPSource.$(OBJ)
MP3_SINK_OBJS = MP3ADURTPSink.$(OBJ)
MPEG_SINK_OBJS = MPEG1or2AudioRTPSink.$(OBJ) $(MP3_SINK_OBJS) MPEG1or2VideoRTPSink.$(OBJ) MPEG4LATMAudioRTPSink.$(OBJ) MPEG4GenericRTPSink.$(OBJ) MPEG4ESVideoRTPSink.$(OBJ)
H263_SINK_OBJS = H263plusVideoRTPSink.$(OBJ)
H264_SINK_OBJS = H264VideoRTPSink.$(OBJ)
DV_SINK_OBJS = DVVideoRTPSink.$(OBJ)
AC3_SINK_OBJS = AC3AudioRTPSink.$(OBJ)

MISC_SOURCE_OBJS = MediaSource.$(OBJ) FramedSource.$(OBJ) FramedFileSource.$(OBJ) FramedFilter.$(OBJ) ByteStreamFileSource.$(OBJ) ByteStreamMultiFileSource.$(OBJ) ByteStreamMemoryBufferSource.$(OBJ) BasicUDPSource.$(OBJ) DeviceSource.$(OBJ) AudioInputDevice.$(OBJ) WAVAudioFileSource.$(OBJ) $(MPEG_SOURCE_OBJS) $(H263_SOURCE_OBJS) $(AC3_SOURCE_OBJS) $(DV_SOURCE_OBJS) JPEGVideoSource.$(OBJ) AMRAudioSource.$(OBJ) AMRAudioFileSource.$(OBJ) InputFile.$(OBJ) StreamReplicator.$(OBJ)
MISC_SINK_OBJS = MediaSink.$(OBJ) FileSink.$(OBJ) BasicUDPSink.$(OBJ) AMRAudioFileSink.$(OBJ) H264VideoFileSink.$(OBJ) $(MPEG_SINK_OBJS) $(H263_SINK_OBJS) $(H264_SINK_OBJS) $(DV_SINK_OBJS) $(AC3_SINK_OBJS) VorbisAudioRTPSink.$(OBJ) VP8VideoRTPSink.$(OBJ) GSMAudioRTPSink.$(OBJ) JPEGVideoRTPSink.$(OBJ) SimpleRTPSink.$(OBJ) AMRAudioRTPSink.$(OBJ) T140TextRTPSink.$(OBJ) TCPStreamSink.$(OBJ) OutputFile.$(OBJ)
MISC_FILTER_OBJS = uLawAudioFilter.$(OBJ)
TRANSPORT_STREAM_TRICK_PLAY_OBJS = MPEG2IndexFromTransportStream.$(OBJ) MPEG2TransportStreamIndexFile.$(OBJ) MPEG2TransportStreamTrickModeFilter.$(OBJ)

RTP_SOURCE_OBJS = RTPSource.$(OBJ) MultiFramedRTPSource.$(OBJ) SimpleRTPSource.$(OBJ) H261VideoRTPSource.$(OBJ) H264VideoRTPSource.$(OBJ) QCELPAudioRTPSource.$(OBJ) AMRAudioRTPSource.$(OBJ) JPEGVideoRTPSource.$(OBJ) VorbisAudioRTPSource.$(OBJ) VP8VideoRTPSource.$(OBJ)
RTP_SINK_OBJS = RTPSink.$(OBJ) MultiFramedRTPSink.$(OBJ) AudioRTPSink.$(OBJ) VideoRTPSink.$(OBJ) TextRTPSink.$(OBJ)
RTP_INTERFACE_OBJS = RTPInterface.$(OBJ)
RTP_OBJS = $(RTP_SOURCE_OBJS) $(RTP_SINK_OBJS) $(RTP_INTERFACE_OBJS)

RTCP_OBJS = RTCP.$(OBJ) rtcp_from_spec.$(OBJ)
RTSP_OBJS = RTSPServer.$(OBJ) RTSPClient.$(OBJ) RTSPCommon.$(OBJ) RTSPServerSupportingHTTPStreaming.$(OBJ)
SIP_OBJS = SIPClient.$(OBJ)

SESSION_OBJS = MediaSession.$(OBJ) ServerMediaSession.$(OBJ) PassiveServerMediaSubsession.$(OBJ) OnDemandServerMediaSubsession.$(OBJ) FileServerMediaSubsession.$(OBJ) MPEG4VideoFileServerMediaSubsession.$(OBJ) H264VideoFileServerMediaSubsession.$(OBJ) H263plusVideoFileServerMediaSubsession.$(OBJ) WAVAudioFileServerMediaSubsession.$(OBJ) AMRAudioFileServerMediaSubsession.$(OBJ) MP3AudioFileServerMediaSubsession.$(OBJ) MPEG1or2VideoFileServerMediaSubsession.$(OBJ) MPEG1or2FileServerDemux.$(OBJ) MPEG1or2DemuxedServerMediaSubsession.$(OBJ) MPEG2TransportFileServerMediaSubsession.$(OBJ) ADTSAudioFileServerMediaSubsession.$(OBJ) DVVideoFileServerMediaSubsession.$(OBJ) AC3AudioFileServerMediaSubsession.$(OBJ) MPEG2TransportUDPServerMediaSubsession.$(OBJ) ProxyServerMediaSession.$(OBJ)

QUICKTIME_OBJS = QuickTimeFileSink.$(OBJ) QuickTimeGenericRTPSource.$(OBJ)
AVI_OBJS = AVIFileSink.$(OBJ)

MATROSKA_FILE_OBJS = MatroskaFile.$(OBJ) MatroskaFileParser.$(OBJ) EBMLNumber.$(OBJ) MatroskaDemuxedTrack.$(OBJ)
MATROSKA_SERVER_MEDIA_SUBSESSION_VIDEO_OBJS = H264VideoMatroskaFileServerMediaSubsession.$(OBJ) VP8VideoMatroskaFileServerMediaSubsession.$(OBJ)
MATROSKA_SERVER_MEDIA_SUBSESSION_AUDIO_OBJS = AACAudioMatroskaFileServerMediaSubsession.$(OBJ) AC3AudioMatroskaFileServerMediaSubsession.$(OBJ) MP3AudioMatroskaFileServerMediaSubsession.$(OBJ) VorbisAudioMatroskaFileServerMediaSubsession.$(OBJ)
MATROSKA_SERVER_MEDIA_SUBSESSION_TEXT_OBJS = T140TextMatroskaFileServerMediaSubsession.$(OBJ)
MATROSKA_SERVER_MEDIA_SUBSESSION_OBJS = $(MATROSKA_SERVER_MEDIA_SUBSESSION_VIDEO_OBJS) $(MATROSKA_SERVER_MEDIA_SUBSESSION_AUDIO_OBJS) $(MATROSKA_SERVER_MEDIA_SUBSESSION_TEXT_OBJS)
MATROSKA_RTSP_SERVER_OBJS = MatroskaFileServerDemux.$(OBJ) $(MATROSKA_SERVER_MEDIA_SUBSESSION_OBJS)
MATROSKA_OBJS = $(MATROSKA_FILE_OBJS) $(MATROSKA_RTSP_SERVER_OBJS)

MISC_OBJS = DarwinInjector.$(OBJ) BitVector.$(OBJ) StreamParser.$(OBJ) DigestAuthentication.$(OBJ) our_md5.$(OBJ) our_md5hl.$(OBJ) Base64.$(OBJ) Locale.$(OBJ)

LIVEMEDIA_LIB_OBJS = Media.$(OBJ) $(MISC_SOURCE_OBJS) $(MISC_SINK_OBJS) $(MISC_FILTER_OBJS) $(RTP_OBJS) $(RTCP_OBJS) $(RTSP_OBJS) $(SIP_OBJS) $(SESSION_OBJS) $(QUICKTIME_OBJS) $(AVI_OBJS) $(TRANSPORT_STREAM_TRICK_PLAY_OBJS) $(MATROSKA_OBJS) $(MISC_OBJS)

$(LIVEMEDIA_LIB): $(LIVEMEDIA_LIB_OBJS) \
    $(PLATFORM_SPECIFIC_LIB_OBJS)
	$(LIBRARY_LINK)$@ $(LIBRARY_LINK_OPTS) \
		$(LIVEMEDIA_LIB_OBJS)

Media.$(CPP):		include/Media.hh
include/Media.hh:	include/liveMedia_version.hh
MediaSource.$(CPP):	include/MediaSource.hh
include/MediaSource.hh:		include/Media.hh
FramedSource.$(CPP):	include/FramedSource.hh
include/FramedSource.hh:	include/MediaSource.hh
FramedFileSource.$(CPP): include/FramedFileSource.hh
include/FramedFileSource.hh:	include/FramedSource.hh
FramedFilter.$(CPP):	include/FramedFilter.hh
include/FramedFilter.hh:	include/FramedSource.hh
RTPSource.$(CPP):	include/RTPSource.hh
include/RTPSource.hh:		include/FramedSource.hh include/RTPInterface.hh
include/RTPInterface.hh:	include/Media.hh
MultiFramedRTPSource.$(CPP):	include/MultiFramedRTPSource.hh
include/MultiFramedRTPSource.hh:	include/RTPSource.hh
SimpleRTPSource.$(CPP):	include/SimpleRTPSource.hh
include/SimpleRTPSource.hh:	include/MultiFramedRTPSource.hh
H261VideoRTPSource.$(CPP):	include/H261VideoRTPSource.hh
include/H261VideoRTPSource.hh:	include/MultiFramedRTPSource.hh
H264VideoRTPSource.$(CPP):      include/H264VideoRTPSource.hh include/Base64.hh
include/H264VideoRTPSource.hh:  include/MultiFramedRTPSource.hh
QCELPAudioRTPSource.$(CPP):	include/QCELPAudioRTPSource.hh include/MultiFramedRTPSource.hh include/FramedFilter.hh
include/QCELPAudioRTPSource.hh:		include/RTPSource.hh
AMRAudioRTPSource.$(CPP):	include/AMRAudioRTPSource.hh include/MultiFramedRTPSource.hh
include/AMRAudioRTPSource.hh:		include/RTPSource.hh include/AMRAudioSource.hh
JPEGVideoRTPSource.$(CPP):	include/JPEGVideoRTPSource.hh
include/JPEGVideoRTPSource.hh:	include/MultiFramedRTPSource.hh
VorbisAudioRTPSource.$(CPP):	include/VorbisAudioRTPSource.hh
include/VorbisAudioRTPSource.hh:	include/MultiFramedRTPSource.hh
VP8VideoRTPSource.$(CPP):	include/VP8VideoRTPSource.hh
include/VP8VideoRTPSource.hh:	include/MultiFramedRTPSource.hh
ByteStreamFileSource.$(CPP):	include/ByteStreamFileSource.hh include/InputFile.hh
include/ByteStreamFileSource.hh:	include/FramedFileSource.hh
ByteStreamMultiFileSource.$(CPP):	include/ByteStreamMultiFileSource.hh
include/ByteStreamMultiFileSource.hh:	include/ByteStreamFileSource.hh
ByteStreamMemoryBufferSource.$(CPP):	include/ByteStreamMemoryBufferSource.hh
include/ByteStreamMemoryBufferSource.hh:	include/FramedSource.hh
BasicUDPSource.$(CPP):		include/BasicUDPSource.hh
include/BasicUDPSource.hh:	include/FramedSource.hh
DeviceSource.$(CPP):	include/DeviceSource.hh
include/DeviceSource.hh:	include/FramedSource.hh
AudioInputDevice.$(CPP):	include/AudioInputDevice.hh
include/AudioInputDevice.hh:	include/FramedSource.hh
WAVAudioFileSource.$(CPP):	include/WAVAudioFileSource.hh include/InputFile.hh
include/WAVAudioFileSource.hh:	include/AudioInputDevice.hh
MPEG1or2Demux.$(CPP):	include/MPEG1or2Demux.hh include/MPEG1or2DemuxedElementaryStream.hh StreamParser.hh
include/MPEG1or2Demux.hh:		include/FramedSource.hh
include/MPEG1or2DemuxedElementaryStream.hh:	include/MPEG1or2Demux.hh
StreamParser.hh:	include/FramedSource.hh
MPEG1or2DemuxedElementaryStream.$(CPP):	include/MPEG1or2DemuxedElementaryStream.hh
MPEGVideoStreamFramer.$(CPP):	MPEGVideoStreamParser.hh
MPEGVideoStreamParser.hh:	StreamParser.hh include/MPEGVideoStreamFramer.hh
include/MPEGVideoStreamFramer.hh:	include/FramedFilter.hh
MPEG1or2VideoStreamFramer.$(CPP):	include/MPEG1or2VideoStreamFramer.hh MPEGVideoStreamParser.hh
include/MPEG1or2VideoStreamFramer.hh:	include/MPEGVideoStreamFramer.hh
MPEG1or2VideoStreamDiscreteFramer.$(CPP):	include/MPEG1or2VideoStreamDiscreteFramer.hh
include/MPEG1or2VideoStreamDiscreteFramer.hh:	include/MPEG1or2VideoStreamFramer.hh
MPEG4VideoStreamFramer.$(CPP):	include/MPEG4VideoStreamFramer.hh MPEGVideoStreamParser.hh include/MPEG4LATMAudioRTPSource.hh
include/MPEG4VideoStreamFramer.hh:	include/MPEGVideoStreamFramer.hh
MPEG4VideoStreamDiscreteFramer.$(CPP):	include/MPEG4VideoStreamDiscreteFramer.hh
include/MPEG4VideoStreamDiscreteFramer.hh:	include/MPEG4VideoStreamFramer.hh
H264VideoStreamFramer.$(CPP):	include/H264VideoStreamFramer.hh MPEGVideoStreamParser.hh BitVector.hh include/H264VideoRTPSource.hh
include/H264VideoStreamFramer.hh:	include/MPEGVideoStreamFramer.hh
H264VideoStreamDiscreteFramer.$(CPP):	include/H264VideoStreamDiscreteFramer.hh
include/H264VideoStreamDiscreteFramer.hh:	include/H264VideoStreamFramer.hh
MPEGVideoStreamParser.$(CPP):	MPEGVideoStreamParser.hh
MPEG1or2AudioStreamFramer.$(CPP):	include/MPEG1or2AudioStreamFramer.hh StreamParser.hh MP3Internals.hh
include/MPEG1or2AudioStreamFramer.hh:	include/FramedFilter.hh
MPEG1or2AudioRTPSource.$(CPP):	include/MPEG1or2AudioRTPSource.hh
include/MPEG1or2AudioRTPSource.hh:	include/MultiFramedRTPSource.hh
MPEG4LATMAudioRTPSource.$(CPP):	include/MPEG4LATMAudioRTPSource.hh
include/MPEG4LATMAudioRTPSource.hh:	include/MultiFramedRTPSource.hh
MPEG4ESVideoRTPSource.$(CPP):	include/MPEG4ESVideoRTPSource.hh
include/MPEG4ESVideoRTPSource.hh:	include/MultiFramedRTPSource.hh
MPEG4GenericRTPSource.$(CPP):	include/MPEG4GenericRTPSource.hh BitVector.hh include/MPEG4LATMAudioRTPSource.hh
include/MPEG4GenericRTPSource.hh:	include/MultiFramedRTPSource.hh
MP3FileSource.$(CPP):	include/MP3FileSource.hh MP3StreamState.hh include/InputFile.hh
include/MP3FileSource.hh:	include/FramedFileSource.hh
MP3StreamState.hh:	MP3Internals.hh
MP3Internals.hh:	BitVector.hh
MP3Transcoder.$(CPP):	include/MP3ADU.hh include/MP3Transcoder.hh
include/MP3ADU.hh:		include/FramedFilter.hh
include/MP3Transcoder.hh:	include/MP3ADU.hh include/MP3ADUTranscoder.hh
include/MP3ADUTranscoder.hh:	include/FramedFilter.hh
MP3ADU.$(CPP):		include/MP3ADU.hh MP3ADUdescriptor.hh MP3Internals.hh
MP3ADUdescriptor.$(CPP):	MP3ADUdescriptor.hh
MP3ADUinterleaving.$(CPP):	include/MP3ADUinterleaving.hh MP3ADUdescriptor.hh
include/MP3ADUinterleaving.hh:	include/FramedFilter.hh
MP3ADUTranscoder.$(CPP):	include/MP3ADUTranscoder.hh MP3Internals.hh
MP3StreamState.$(CPP):	MP3StreamState.hh include/InputFile.hh
MP3Internals.$(CPP):	MP3InternalsHuffman.hh
MP3InternalsHuffman.hh:	MP3Internals.hh
MP3InternalsHuffman.$(CPP):	MP3InternalsHuffman.hh
MP3InternalsHuffmanTable.$(CPP):	MP3InternalsHuffman.hh
MP3ADURTPSource.$(CPP):	include/MP3ADURTPSource.hh MP3ADUdescriptor.hh
include/MP3ADURTPSource.hh:	include/MultiFramedRTPSource.hh
MPEG1or2VideoRTPSource.$(CPP):	include/MPEG1or2VideoRTPSource.hh
include/MPEG1or2VideoRTPSource.hh:	include/MultiFramedRTPSource.hh
MPEG2TransportStreamMultiplexor.$(CPP):	include/MPEG2TransportStreamMultiplexor.hh
include/MPEG2TransportStreamMultiplexor.hh:	include/FramedSource.hh include/MPEG1or2Demux.hh
MPEG2TransportStreamFromPESSource.$(CPP):	include/MPEG2TransportStreamFromPESSource.hh
include/MPEG2TransportStreamFromPESSource.hh:	include/MPEG2TransportStreamMultiplexor.hh include/MPEG1or2DemuxedElementaryStream.hh
MPEG2TransportStreamFromESSource.$(CPP):	include/MPEG2TransportStreamFromESSource.hh
include/MPEG2TransportStreamFromESSource.hh:	include/MPEG2TransportStreamMultiplexor.hh
MPEG2TransportStreamFramer.$(CPP):	include/MPEG2TransportStreamFramer.hh
include/MPEG2TransportStreamFramer.hh:	include/FramedFilter.hh include/MPEG2TransportStreamIndexFile.hh
ADTSAudioFileSource.$(CPP):	include/ADTSAudioFileSource.hh include/InputFile.hh
include/ADTSAudioFileSource.hh:	include/FramedFileSource.hh
H263plusVideoRTPSource.$(CPP):	include/H263plusVideoRTPSource.hh
include/H263plusVideoRTPSource.hh:	include/MultiFramedRTPSource.hh
H263plusVideoStreamFramer.$(CPP):	include/H263plusVideoStreamFramer.hh H263plusVideoStreamParser.hh
include/H263plusVideoStreamFramer.hh:	include/FramedFilter.hh
H263plusVideoStreamParser.hh:	StreamParser.hh
H263plusVideoStreamParser.$(CPP):	H263plusVideoStreamParser.hh include/H263plusVideoStreamFramer.hh
AC3AudioStreamFramer.$(CPP):	include/AC3AudioStreamFramer.hh StreamParser.hh
include/AC3AudioStreamFramer.hh:	include/FramedFilter.hh
AC3AudioRTPSource.$(CPP):	include/AC3AudioRTPSource.hh
include/AC3AudioRTPSource.hh:	include/MultiFramedRTPSource.hh
DVVideoRTPSource.$(CPP):	include/DVVideoRTPSource.hh
include/DVVideoRTPSource.hh:	include/MultiFramedRTPSource.hh
JPEGVideoSource.$(CPP):		include/JPEGVideoSource.hh
include/JPEGVideoSource.hh:	include/FramedSource.hh
AMRAudioSource.$(CPP):	include/AMRAudioSource.hh
include/AMRAudioSource.hh:	include/FramedSource.hh
AMRAudioFileSource.$(CPP):	include/AMRAudioFileSource.hh include/InputFile.hh
include/AMRAudioFileSource.hh:	include/AMRAudioSource.hh
InputFile.$(CPP):		include/InputFile.hh
StreamReplicator.$(CPP):	include/StreamReplicator.hh
include/StreamReplicator.hh:	include/FramedSource.hh
MediaSink.$(CPP):	include/MediaSink.hh
include/MediaSink.hh:		include/FramedSource.hh
FileSink.$(CPP):	include/FileSink.hh include/OutputFile.hh
include/FileSink.hh:		include/MediaSink.hh
BasicUDPSink.$(CPP):	include/BasicUDPSink.hh
include/BasicUDPSink.hh:	include/MediaSink.hh
AMRAudioFileSink.$(CPP):	include/AMRAudioFileSink.hh include/AMRAudioSource.hh include/OutputFile.hh
include/AMRAudioFileSink.hh:	include/FileSink.hh
H264VideoFileSink.$(CPP):       include/H264VideoFileSink.hh include/OutputFile.hh include/H264VideoRTPSource.hh
include/H264VideoFileSink.hh:   include/FileSink.hh
RTPSink.$(CPP):		include/RTPSink.hh
include/RTPSink.hh:		include/MediaSink.hh include/RTPInterface.hh
MultiFramedRTPSink.$(CPP):	include/MultiFramedRTPSink.hh
include/MultiFramedRTPSink.hh:		include/RTPSink.hh
AudioRTPSink.$(CPP):		include/AudioRTPSink.hh
include/AudioRTPSink.hh:	include/MultiFramedRTPSink.hh
VideoRTPSink.$(CPP):		include/VideoRTPSink.hh
include/VideoRTPSink.hh:	include/MultiFramedRTPSink.hh
TextRTPSink.$(CPP):		include/TextRTPSink.hh
include/TextRTPSink.hh:		include/MultiFramedRTPSink.hh
RTPInterface.$(CPP):		include/RTPInterface.hh
MPEG1or2AudioRTPSink.$(CPP):	include/MPEG1or2AudioRTPSink.hh
include/MPEG1or2AudioRTPSink.hh:	include/AudioRTPSink.hh
MP3ADURTPSink.$(CPP):	include/MP3ADURTPSink.hh
include/MP3ADURTPSink.hh:	include/AudioRTPSink.hh
MPEG1or2VideoRTPSink.$(CPP):	include/MPEG1or2VideoRTPSink.hh include/MPEG1or2VideoStreamFramer.hh
include/MPEG1or2VideoRTPSink.hh:	include/VideoRTPSink.hh
MPEG4LATMAudioRTPSink.$(CPP):	include/MPEG4LATMAudioRTPSink.hh
include/MPEG4LATMAudioRTPSink.hh:	include/AudioRTPSink.hh
MPEG4GenericRTPSink.$(CPP):	include/MPEG4GenericRTPSink.hh include/Locale.hh
include/MPEG4GenericRTPSink.hh:	include/MultiFramedRTPSink.hh
MPEG4ESVideoRTPSink.$(CPP):	include/MPEG4ESVideoRTPSink.hh include/MPEG4VideoStreamFramer.hh include/MPEG4LATMAudioRTPSource.hh
include/MPEG4ESVideoRTPSink.hh: include/VideoRTPSink.hh
H263plusVideoRTPSink.$(CPP):	include/H263plusVideoRTPSink.hh
include/H263plusVideoRTPSink.hh:	include/VideoRTPSink.hh
H264VideoRTPSink.$(CPP):	include/H264VideoRTPSink.hh include/H264VideoStreamFramer.hh include/Base64.hh include/H264VideoRTPSource.hh
include/H264VideoRTPSink.hh:	include/VideoRTPSink.hh include/FramedFilter.hh
DVVideoRTPSink.$(CPP):	include/DVVideoRTPSink.hh
include/DVVideoRTPSink.hh:	include/VideoRTPSink.hh include/DVVideoStreamFramer.hh
include/DVVideoStreamFramer.hh:	include/FramedFilter.hh
AC3AudioRTPSink.$(CPP):		include/AC3AudioRTPSink.hh
include/AC3AudioRTPSink.hh:	include/AudioRTPSink.hh
VorbisAudioRTPSink.$(CPP):	include/VorbisAudioRTPSink.hh include/Base64.hh
include/VorbisAudioRTPSink.hh:	include/AudioRTPSink.hh
VP8VideoRTPSink.$(CPP):		include/VP8VideoRTPSink.hh
include/VP8VideoRTPSink.hh:	include/VideoRTPSink.hh
GSMAudioRTPSink.$(CPP):		include/GSMAudioRTPSink.hh
include/GSMAudioRTPSink.hh:	include/AudioRTPSink.hh
JPEGVideoRTPSink.$(CPP):	include/JPEGVideoRTPSink.hh include/JPEGVideoSource.hh
include/JPEGVideoRTPSink.hh:	include/VideoRTPSink.hh
SimpleRTPSink.$(CPP):		include/SimpleRTPSink.hh
include/SimpleRTPSink.hh:	include/MultiFramedRTPSink.hh
AMRAudioRTPSink.$(CPP):		include/AMRAudioRTPSink.hh include/AMRAudioSource.hh
include/AMRAudioRTPSink.hh:	include/AudioRTPSink.hh
T140TextRTPSink.$(CPP):		include/T140TextRTPSink.hh
include/T140TextRTPSink.hh:	include/TextRTPSink.hh include/FramedFilter.hh
TCPStreamSink.$(CPP):		include/TCPStreamSink.hh
include/TCPStreamSink.hh:	include/MediaSink.hh
OutputFile.$(CPP):		include/OutputFile.hh
uLawAudioFilter.$(CPP):		include/uLawAudioFilter.hh
include/uLawAudioFilter.hh:	include/FramedFilter.hh
MPEG2IndexFromTransportStream.$(CPP):	include/MPEG2IndexFromTransportStream.hh
include/MPEG2IndexFromTransportStream.hh:	include/FramedFilter.hh
MPEG2TransportStreamIndexFile.$(CPP):	include/MPEG2TransportStreamIndexFile.hh include/InputFile.hh
include/MPEG2TransportStreamIndexFile.hh:	include/Media.hh
MPEG2TransportStreamTrickModeFilter.$(CPP):	include/MPEG2TransportStreamTrickModeFilter.hh include/ByteStreamFileSource.hh
include/MPEG2TransportStreamTrickModeFilter.hh:	include/FramedFilter.hh include/MPEG2TransportStreamIndexFile.hh
RTCP.$(CPP):		include/RTCP.hh rtcp_from_spec.h
include/RTCP.hh:		include/RTPSink.hh include/RTPSource.hh
rtcp_from_spec.$(C):	rtcp_from_spec.h
RTSPServer.$(CPP):	include/RTSPServer.hh include/RTSPCommon.hh include/Base64.hh
include/RTSPServer.hh:		include/ServerMediaSession.hh include/DigestAuthentication.hh include/RTSPCommon.hh
include/ServerMediaSession.hh:	include/Media.hh include/FramedSource.hh include/RTPInterface.hh
RTSPClient.$(CPP):	include/RTSPClient.hh  include/RTSPCommon.hh include/Base64.hh include/Locale.hh our_md5.h
include/RTSPClient.hh:		include/MediaSession.hh include/DigestAuthentication.hh
RTSPCommon.$(CPP):	include/RTSPCommon.hh include/Locale.hh
RTSPServerSupportingHTTPStreaming.$(CPP):	include/RTSPServerSupportingHTTPStreaming.hh include/RTSPCommon.hh
include/RTSPServerSupportingHTTPStreaming.hh:	include/RTSPServer.hh include/ByteStreamMemoryBufferSource.hh include/TCPStreamSink.hh
SIPClient.$(CPP):	include/SIPClient.hh
include/SIPClient.hh:		include/MediaSession.hh include/DigestAuthentication.hh
MediaSession.$(CPP):	include/liveMedia.hh include/Locale.hh
include/MediaSession.hh:	include/RTCP.hh include/FramedFilter.hh
ServerMediaSession.$(CPP):	include/ServerMediaSession.hh
PassiveServerMediaSubsession.$(CPP):	include/PassiveServerMediaSubsession.hh
include/PassiveServerMediaSubsession.hh:	include/ServerMediaSession.hh include/RTPSink.hh include/RTCP.hh
OnDemandServerMediaSubsession.$(CPP):	include/OnDemandServerMediaSubsession.hh
include/OnDemandServerMediaSubsession.hh:	include/ServerMediaSession.hh include/RTPSink.hh include/BasicUDPSink.hh include/RTCP.hh
FileServerMediaSubsession.$(CPP):	include/FileServerMediaSubsession.hh
include/FileServerMediaSubsession.hh:	include/OnDemandServerMediaSubsession.hh
MPEG4VideoFileServerMediaSubsession.$(CPP):	include/MPEG4VideoFileServerMediaSubsession.hh include/MPEG4ESVideoRTPSink.hh include/ByteStreamFileSource.hh include/MPEG4VideoStreamFramer.hh
include/MPEG4VideoFileServerMediaSubsession.hh:	include/FileServerMediaSubsession.hh
H264VideoFileServerMediaSubsession.$(CPP):	include/H264VideoFileServerMediaSubsession.hh include/H264VideoRTPSink.hh include/ByteStreamFileSource.hh include/H264VideoStreamFramer.hh
include/H264VideoFileServerMediaSubsession.hh:	include/FileServerMediaSubsession.hh
H263plusVideoFileServerMediaSubsession.$(CPP):	include/H263plusVideoFileServerMediaSubsession.hh include/H263plusVideoRTPSink.hh include/ByteStreamFileSource.hh include/H263plusVideoStreamFramer.hh
include/H263plusVideoFileServerMediaSubsession.hh:	include/FileServerMediaSubsession.hh
WAVAudioFileServerMediaSubsession.$(CPP):	include/WAVAudioFileServerMediaSubsession.hh include/WAVAudioFileSource.hh include/uLawAudioFilter.hh include/SimpleRTPSink.hh
include/WAVAudioFileServerMediaSubsession.hh:	include/FileServerMediaSubsession.hh
AMRAudioFileServerMediaSubsession.$(CPP):	include/AMRAudioFileServerMediaSubsession.hh include/AMRAudioRTPSink.hh include/AMRAudioFileSource.hh
include/AMRAudioFileServerMediaSubsession.hh:	include/FileServerMediaSubsession.hh
MP3AudioFileServerMediaSubsession.$(CPP):	include/MP3AudioFileServerMediaSubsession.hh include/MPEG1or2AudioRTPSink.hh include/MP3ADURTPSink.hh include/MP3FileSource.hh include/MP3ADU.hh
include/MP3AudioFileServerMediaSubsession.hh:	include/FileServerMediaSubsession.hh include/MP3ADUinterleaving.hh
MPEG1or2VideoFileServerMediaSubsession.$(CPP):	include/MPEG1or2VideoFileServerMediaSubsession.hh include/MPEG1or2VideoRTPSink.hh include/ByteStreamFileSource.hh include/MPEG1or2VideoStreamFramer.hh
include/MPEG1or2VideoFileServerMediaSubsession.hh:	include/FileServerMediaSubsession.hh
MPEG1or2FileServerDemux.$(CPP):	include/MPEG1or2FileServerDemux.hh include/MPEG1or2DemuxedServerMediaSubsession.hh include/ByteStreamFileSource.hh
include/MPEG1or2FileServerDemux.hh:	include/ServerMediaSession.hh include/MPEG1or2DemuxedElementaryStream.hh
MPEG1or2DemuxedServerMediaSubsession.$(CPP): include/MPEG1or2DemuxedServerMediaSubsession.hh include/MPEG1or2AudioStreamFramer.hh include/MPEG1or2AudioRTPSink.hh include/MPEG1or2VideoStreamFramer.hh include/MPEG1or2VideoRTPSink.hh include/AC3AudioStreamFramer.hh include/AC3AudioRTPSink.hh include/ByteStreamFileSource.hh
include/MPEG1or2DemuxedServerMediaSubsession.hh: include/OnDemandServerMediaSubsession.hh include/MPEG1or2FileServerDemux.hh
MPEG2TransportFileServerMediaSubsession.$(CPP):	include/MPEG2TransportFileServerMediaSubsession.hh include/SimpleRTPSink.hh
include/MPEG2TransportFileServerMediaSubsession.hh:	include/FileServerMediaSubsession.hh include/MPEG2TransportStreamFramer.hh include/ByteStreamFileSource.hh include/MPEG2TransportStreamTrickModeFilter.hh include/MPEG2TransportStreamFromESSource.hh
ADTSAudioFileServerMediaSubsession.$(CPP):	include/ADTSAudioFileServerMediaSubsession.hh include/ADTSAudioFileSource.hh include/MPEG4GenericRTPSink.hh
include/ADTSAudioFileServerMediaSubsession.hh:	include/FileServerMediaSubsession.hh
DVVideoFileServerMediaSubsession.$(CPP):	include/DVVideoFileServerMediaSubsession.hh include/DVVideoRTPSink.hh include/ByteStreamFileSource.hh include/DVVideoStreamFramer.hh
include/DVVideoFileServerMediaSubsession.hh:	include/FileServerMediaSubsession.hh
AC3AudioFileServerMediaSubsession.$(CPP):	include/AC3AudioFileServerMediaSubsession.hh include/AC3AudioRTPSink.hh include/ByteStreamFileSource.hh include/AC3AudioStreamFramer.hh
include/AC3AudioFileServerMediaSubsession.hh:	include/FileServerMediaSubsession.hh
MPEG2TransportUDPServerMediaSubsession.$(CPP):	include/MPEG2TransportUDPServerMediaSubsession.hh include/BasicUDPSource.hh include/SimpleRTPSource.hh include/MPEG2TransportStreamFramer.hh include/SimpleRTPSink.hh
include/MPEG2TransportUDPServerMediaSubsession.hh:	include/OnDemandServerMediaSubsession.hh
ProxyServerMediaSession.$(CPP):		include/liveMedia.hh include/RTSPCommon.hh
QuickTimeFileSink.$(CPP):	include/QuickTimeFileSink.hh include/InputFile.hh include/OutputFile.hh include/QuickTimeGenericRTPSource.hh include/H263plusVideoRTPSource.hh include/MPEG4GenericRTPSource.hh include/MPEG4LATMAudioRTPSource.hh
include/QuickTimeFileSink.hh:	include/MediaSession.hh
QuickTimeGenericRTPSource.$(CPP):	include/QuickTimeGenericRTPSource.hh
include/QuickTimeGenericRTPSource.hh:	include/MultiFramedRTPSource.hh
AVIFileSink.$(CPP):	include/AVIFileSink.hh include/InputFile.hh include/OutputFile.hh
include/AVIFileSink.hh:	include/MediaSession.hh
MatroskaFile.$(CPP): MatroskaFileParser.hh MatroskaDemuxedTrack.hh include/ByteStreamFileSource.hh
MatroskaFileParser.hh:	StreamParser.hh include/MatroskaFile.hh EBMLNumber.hh
include/MatroskaFile.hh: include/Media.hh
MatroskaDemuxedTrack.hh:	include/FramedSource.hh
MatroskaFileParser.$(CPP): MatroskaFileParser.hh MatroskaDemuxedTrack.hh include/ByteStreamFileSource.hh
EBMLNumber.$(CPP): EBMLNumber.hh
MatroskaDemuxedTrack.$(CPP): MatroskaDemuxedTrack.hh include/MatroskaFile.hh
H264VideoMatroskaFileServerMediaSubsession.$(CPP): H264VideoMatroskaFileServerMediaSubsession.hh include/H264VideoStreamDiscreteFramer.hh
H264VideoMatroskaFileServerMediaSubsession.hh: include/H264VideoFileServerMediaSubsession.hh include/MatroskaFileServerDemux.hh MatroskaDemuxedTrack.hh
VP8VideoMatroskaFileServerMediaSubsession.$(CPP): VP8VideoMatroskaFileServerMediaSubsession.hh include/VP8VideoRTPSink.hh
VP8VideoMatroskaFileServerMediaSubsession.hh: include/FileServerMediaSubsession.hh include/MatroskaFileServerDemux.hh MatroskaDemuxedTrack.hh
AACAudioMatroskaFileServerMediaSubsession.$(CPP): AACAudioMatroskaFileServerMediaSubsession.hh include/MPEG4GenericRTPSink.hh
AACAudioMatroskaFileServerMediaSubsession.hh: include/FileServerMediaSubsession.hh include/MatroskaFileServerDemux.hh MatroskaDemuxedTrack.hh
AC3AudioMatroskaFileServerMediaSubsession.$(CPP): AC3AudioMatroskaFileServerMediaSubsession.hh include/AC3AudioRTPSink.hh
AC3AudioMatroskaFileServerMediaSubsession.hh: include/FileServerMediaSubsession.hh include/MatroskaFileServerDemux.hh MatroskaDemuxedTrack.hh
VorbisAudioMatroskaFileServerMediaSubsession.$(CPP): VorbisAudioMatroskaFileServerMediaSubsession.hh include/VorbisAudioRTPSink.hh
VorbisAudioMatroskaFileServerMediaSubsession.hh: include/FileServerMediaSubsession.hh include/MatroskaFileServerDemux.hh MatroskaDemuxedTrack.hh
MP3AudioMatroskaFileServerMediaSubsession.$(CPP): MP3AudioMatroskaFileServerMediaSubsession.hh
MP3AudioMatroskaFileServerMediaSubsession.hh: include/MP3AudioFileServerMediaSubsession.hh include/MatroskaFileServerDemux.hh MatroskaDemuxedTrack.hh
T140TextMatroskaFileServerMediaSubsession.$(CPP): T140TextMatroskaFileServerMediaSubsession.hh include/T140TextRTPSink.hh
T140TextMatroskaFileServerMediaSubsession.hh: include/FileServerMediaSubsession.hh include/MatroskaFileServerDemux.hh MatroskaDemuxedTrack.hh
MatroskaFileServerDemux.$(CPP): include/MatroskaFileServerDemux.hh H264VideoMatroskaFileServerMediaSubsession.hh VP8VideoMatroskaFileServerMediaSubsession.hh AACAudioMatroskaFileServerMediaSubsession.hh AC3AudioMatroskaFileServerMediaSubsession.hh VorbisAudioMatroskaFileServerMediaSubsession.hh MP3AudioMatroskaFileServerMediaSubsession.hh T140TextMatroskaFileServerMediaSubsession.hh
include/MatroskaFileServerDemux.hh: include/ServerMediaSession.hh include/MatroskaFile.hh
DarwinInjector.$(CPP):	include/DarwinInjector.hh
include/DarwinInjector.hh:	include/RTSPClient.hh include/RTCP.hh
BitVector.$(CPP):	BitVector.hh
StreamParser.$(CPP):	StreamParser.hh
DigestAuthentication.$(CPP):	include/DigestAuthentication.hh our_md5.h
our_md5.$(C):		our_md5.h
our_md5hl.$(C):		our_md5.h
Base64.$(CPP):	include/Base64.hh
Locale.$(CPP):	include/Locale.hh

include/liveMedia.hh:: include/MPEG1or2AudioRTPSink.hh include/MP3ADURTPSink.hh include/MPEG1or2VideoRTPSink.hh include/MPEG4ESVideoRTPSink.hh include/BasicUDPSink.hh include/AMRAudioFileSink.hh include/H264VideoFileSink.hh include/GSMAudioRTPSink.hh include/H263plusVideoRTPSink.hh include/H264VideoRTPSink.hh include/DVVideoRTPSource.hh include/DVVideoRTPSink.hh include/DVVideoStreamFramer.hh include/H264VideoStreamDiscreteFramer.hh include/JPEGVideoRTPSink.hh include/SimpleRTPSink.hh include/uLawAudioFilter.hh include/MPEG2IndexFromTransportStream.hh include/MPEG2TransportStreamTrickModeFilter.hh include/ByteStreamMultiFileSource.hh include/ByteStreamMemoryBufferSource.hh include/BasicUDPSource.hh include/SimpleRTPSource.hh include/MPEG1or2AudioRTPSource.hh include/MPEG4LATMAudioRTPSource.hh include/MPEG4LATMAudioRTPSink.hh include/MPEG4ESVideoRTPSource.hh include/MPEG4GenericRTPSource.hh include/MP3ADURTPSource.hh include/QCELPAudioRTPSource.hh include/AMRAudioRTPSource.hh include/JPEGVideoRTPSource.hh include/JPEGVideoSource.hh include/MPEG1or2VideoRTPSource.hh include/VorbisAudioRTPSource.hh include/VP8VideoRTPSource.hh

include/liveMedia.hh::	include/MPEG2TransportStreamFromPESSource.hh include/MPEG2TransportStreamFromESSource.hh include/MPEG2TransportStreamFramer.hh include/ADTSAudioFileSource.hh include/H261VideoRTPSource.hh include/H263plusVideoRTPSource.hh include/H264VideoRTPSource.hh include/MP3FileSource.hh include/MP3ADU.hh include/MP3ADUinterleaving.hh include/MP3Transcoder.hh include/MPEG1or2DemuxedElementaryStream.hh include/MPEG1or2AudioStreamFramer.hh include/MPEG1or2VideoStreamDiscreteFramer.hh include/MPEG4VideoStreamDiscreteFramer.hh include/H263plusVideoStreamFramer.hh include/AC3AudioStreamFramer.hh include/AC3AudioRTPSource.hh include/AC3AudioRTPSink.hh include/VorbisAudioRTPSink.hh include/VP8VideoRTPSink.hh include/MPEG4GenericRTPSink.hh include/DeviceSource.hh include/AudioInputDevice.hh include/WAVAudioFileSource.hh include/StreamReplicator.hh

include/liveMedia.hh:: include/RTSPServerSupportingHTTPStreaming.hh include/RTSPClient.hh include/SIPClient.hh include/QuickTimeFileSink.hh include/QuickTimeGenericRTPSource.hh include/AVIFileSink.hh include/PassiveServerMediaSubsession.hh include/MPEG4VideoFileServerMediaSubsession.hh include/H264VideoFileServerMediaSubsession.hh include/WAVAudioFileServerMediaSubsession.hh include/AMRAudioFileServerMediaSubsession.hh include/AMRAudioFileSource.hh include/AMRAudioRTPSink.hh include/T140TextRTPSink.hh include/TCPStreamSink.hh include/MP3AudioFileServerMediaSubsession.hh include/MPEG1or2VideoFileServerMediaSubsession.hh include/MPEG1or2FileServerDemux.hh include/MPEG2TransportFileServerMediaSubsession.hh include/H263plusVideoFileServerMediaSubsession.hh include/ADTSAudioFileServerMediaSubsession.hh include/DVVideoFileServerMediaSubsession.hh include/AC3AudioFileServerMediaSubsession.hh include/MPEG2TransportUDPServerMediaSubsession.hh include/MatroskaFileServerDemux.hh include/ProxyServerMediaSession.hh include/DarwinInjector.hh

clean:
	-rm -rf *.$(OBJ) $(ALL) core *.core *~ include/*~

install: install1 $(INSTALL2)
install1: $(LIVEMEDIA_LIB)
	 install -d $(DESTDIR)$(PREFIX)/include/liveMedia $(DESTDIR)$(LIBDIR)
	 install -m 644 include/*.hh $(DESTDIR)$(PREFIX)/include/liveMedia
	 install -m 644 $(LIVEMEDIA_LIB) $(DESTDIR)$(LIBDIR)
install_shared_libraries: $(LIVEMEDIA_LIB)
	 ln -s $(NAME).$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)/$(NAME).$(SHORT_LIB_SUFFIX)
	 ln -s $(NAME).$(LIB_SUFFIX) $(DESTDIR)$(LIBDIR)/$(NAME).so

##### Any additional, platform-specific rules come here:
