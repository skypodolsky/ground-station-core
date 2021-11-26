#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Leanspace + GNU Radio
# GNU Radio version: v3.8.3.1-2-g18f86220

from gnuradio import analog
from gnuradio import blocks
from gnuradio import filter
from gnuradio.filter import firdes
from gnuradio import gr
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
import osmosdr
import time
import satellites.components.deframers
import satellites.components.demodulators
try:
    from xmlrpc.server import SimpleXMLRPCServer
except ImportError:
    from SimpleXMLRPCServer import SimpleXMLRPCServer
import threading
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser
try:
    import configparser
except ImportError:
    import ConfigParser as configparser


class fmDemod(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "Leanspace + GNU Radio")

        ##################################################
        # Variables
        ##################################################
        self.sampRate = sampRate = 480000
        self._main_Result_File_Prefix_Str_config = configparser.ConfigParser()
        self._main_Result_File_Prefix_Str_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: main_Result_File_Prefix_Str = self._main_Result_File_Prefix_Str_config.get('main', 'main_Result_File_Prefix')
        except: main_Result_File_Prefix_Str = 'ERROR_PREFIX'
        self.main_Result_File_Prefix_Str = main_Result_File_Prefix_Str
        self._sdr_LNA_Gain_Int_config = configparser.ConfigParser()
        self._sdr_LNA_Gain_Int_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: sdr_LNA_Gain_Int = self._sdr_LNA_Gain_Int_config.getint('sdr', 'sdr_LNA_Gain')
        except: sdr_LNA_Gain_Int = 14
        self.sdr_LNA_Gain_Int = sdr_LNA_Gain_Int
        self._sdr_IF_Gain_Int_config = configparser.ConfigParser()
        self._sdr_IF_Gain_Int_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: sdr_IF_Gain_Int = self._sdr_IF_Gain_Int_config.getint('sdr', 'sdr_IF_Gain')
        except: sdr_IF_Gain_Int = 40
        self.sdr_IF_Gain_Int = sdr_IF_Gain_Int
        self._sdr_Freq_Int_config = configparser.ConfigParser()
        self._sdr_Freq_Int_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: sdr_Freq_Int = self._sdr_Freq_Int_config.getint('sdr', 'sdr_Freq')
        except: sdr_Freq_Int = 145935000
        self.sdr_Freq_Int = sdr_Freq_Int
        self._sdr_BB_Gain_Int_config = configparser.ConfigParser()
        self._sdr_BB_Gain_Int_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: sdr_BB_Gain_Int = self._sdr_BB_Gain_Int_config.getint('sdr', 'sdr_BB_Gain')
        except: sdr_BB_Gain_Int = 40
        self.sdr_BB_Gain_Int = sdr_BB_Gain_Int
        self._modulation_Type_Int_config = configparser.ConfigParser()
        self._modulation_Type_Int_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: modulation_Type_Int = self._modulation_Type_Int_config.getint('modulation', 'modulation_Type')
        except: modulation_Type_Int = 0
        self.modulation_Type_Int = modulation_Type_Int
        self._modulation_FSK_Sub_Audio_Bool_config = configparser.ConfigParser()
        self._modulation_FSK_Sub_Audio_Bool_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: modulation_FSK_Sub_Audio_Bool = self._modulation_FSK_Sub_Audio_Bool_config.getboolean('modulation', 'modulation_FSK_Sub_Audio')
        except: modulation_FSK_Sub_Audio_Bool = False
        self.modulation_FSK_Sub_Audio_Bool = modulation_FSK_Sub_Audio_Bool
        self._modulation_Deframer_Type_Int_config = configparser.ConfigParser()
        self._modulation_Deframer_Type_Int_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: modulation_Deframer_Type_Int = self._modulation_Deframer_Type_Int_config.getint('modulation', 'modulation_Deframer_Type')
        except: modulation_Deframer_Type_Int = 0
        self.modulation_Deframer_Type_Int = modulation_Deframer_Type_Int
        self._modulation_Baud_Rate_Int_config = configparser.ConfigParser()
        self._modulation_Baud_Rate_Int_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: modulation_Baud_Rate_Int = self._modulation_Baud_Rate_Int_config.getint('modulation', 'modulation_Baud_Rate')
        except: modulation_Baud_Rate_Int = 9600
        self.modulation_Baud_Rate_Int = modulation_Baud_Rate_Int
        self._modulation_BPSK_Manchester_Bool_config = configparser.ConfigParser()
        self._modulation_BPSK_Manchester_Bool_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: modulation_BPSK_Manchester_Bool = self._modulation_BPSK_Manchester_Bool_config.getboolean('modulation', 'modulation_BPSK_Manchester')
        except: modulation_BPSK_Manchester_Bool = False
        self.modulation_BPSK_Manchester_Bool = modulation_BPSK_Manchester_Bool
        self._modulation_BPSK_Differential_Bool_config = configparser.ConfigParser()
        self._modulation_BPSK_Differential_Bool_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: modulation_BPSK_Differential_Bool = self._modulation_BPSK_Differential_Bool_config.getboolean('modulation', 'modulation_BPSK_Differential')
        except: modulation_BPSK_Differential_Bool = True
        self.modulation_BPSK_Differential_Bool = modulation_BPSK_Differential_Bool
        self._modulation_AFSK_Deviation_Int_config = configparser.ConfigParser()
        self._modulation_AFSK_Deviation_Int_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: modulation_AFSK_Deviation_Int = self._modulation_AFSK_Deviation_Int_config.getint('modulation', 'modulation_AFSK_Deviation')
        except: modulation_AFSK_Deviation_Int = -1200
        self.modulation_AFSK_Deviation_Int = modulation_AFSK_Deviation_Int
        self._modulation_AFSK_Audio_Carrier_Freq_Int_config = configparser.ConfigParser()
        self._modulation_AFSK_Audio_Carrier_Freq_Int_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: modulation_AFSK_Audio_Carrier_Freq_Int = self._modulation_AFSK_Audio_Carrier_Freq_Int_config.getint('modulation', 'modulation_AFSK_Audio_Carrier_Freq')
        except: modulation_AFSK_Audio_Carrier_Freq_Int = 3600
        self.modulation_AFSK_Audio_Carrier_Freq_Int = modulation_AFSK_Audio_Carrier_Freq_Int
        self._main_Network_Stream_Port_Int_config = configparser.ConfigParser()
        self._main_Network_Stream_Port_Int_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: main_Network_Stream_Port_Int = self._main_Network_Stream_Port_Int_config.getint('main', 'main_Network_Stream_Port')
        except: main_Network_Stream_Port_Int = 8080
        self.main_Network_Stream_Port_Int = main_Network_Stream_Port_Int
        self._main_Network_Stream_Address_Str_config = configparser.ConfigParser()
        self._main_Network_Stream_Address_Str_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: main_Network_Stream_Address_Str = self._main_Network_Stream_Address_Str_config.get('main', 'main_Network_Stream_Address')
        except: main_Network_Stream_Address_Str = '127.0.0.1'
        self.main_Network_Stream_Address_Str = main_Network_Stream_Address_Str
        self._main_LPF_Cutoff_Freq_Int_config = configparser.ConfigParser()
        self._main_LPF_Cutoff_Freq_Int_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: main_LPF_Cutoff_Freq_Int = self._main_LPF_Cutoff_Freq_Int_config.getint('main', 'main_LPF_Cutoff_Freq')
        except: main_LPF_Cutoff_Freq_Int = sampRate - 10000
        self.main_LPF_Cutoff_Freq_Int = main_LPF_Cutoff_Freq_Int
        self.freqOffset = freqOffset = 11000
        self.fileNameRes = fileNameRes = main_Result_File_Prefix_Str + ".dat"
        self.fileNameRaw = fileNameRaw = main_Result_File_Prefix_Str + ".raw"
        self._deframer_Syncword_Threshold_Int_config = configparser.ConfigParser()
        self._deframer_Syncword_Threshold_Int_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: deframer_Syncword_Threshold_Int = self._deframer_Syncword_Threshold_Int_config.getint('deframer', 'deframer_Syncword_Threshold')
        except: deframer_Syncword_Threshold_Int = 4
        self.deframer_Syncword_Threshold_Int = deframer_Syncword_Threshold_Int
        self._deframer_Short_Frames_Bool_config = configparser.ConfigParser()
        self._deframer_Short_Frames_Bool_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: deframer_Short_Frames_Bool = self._deframer_Short_Frames_Bool_config.getboolean('deframer', 'deframer_Short_Frames')
        except: deframer_Short_Frames_Bool = False
        self.deframer_Short_Frames_Bool = deframer_Short_Frames_Bool
        self._deframer_G3RUH_Bool_config = configparser.ConfigParser()
        self._deframer_G3RUH_Bool_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: deframer_G3RUH_Bool = self._deframer_G3RUH_Bool_config.getboolean('deframer', 'deframer_G3RUH')
        except: deframer_G3RUH_Bool = False
        self.deframer_G3RUH_Bool = deframer_G3RUH_Bool
        self._deframer_CRC16_Bool_config = configparser.ConfigParser()
        self._deframer_CRC16_Bool_config.read('/home/stanislavb/sdr_prototypes/master/default')
        try: deframer_CRC16_Bool = self._deframer_CRC16_Bool_config.getboolean('deframer', 'deframer_CRC16')
        except: deframer_CRC16_Bool = False
        self.deframer_CRC16_Bool = deframer_CRC16_Bool

        ##################################################
        # Blocks
        ##################################################
        self.xmlrpc_server_0 = SimpleXMLRPCServer(('localhost', 7777), allow_none=True)
        self.xmlrpc_server_0.register_instance(self)
        self.xmlrpc_server_0_thread = threading.Thread(target=self.xmlrpc_server_0.serve_forever)
        self.xmlrpc_server_0_thread.daemon = True
        self.xmlrpc_server_0_thread.start()
        self.satellites_u482c_deframer_0 = satellites.components.deframers.u482c_deframer(syncword_threshold = deframer_Syncword_Threshold_Int, options="")
        self.satellites_fsk_demodulator_0 = satellites.components.demodulators.fsk_demodulator(baudrate = modulation_Baud_Rate_Int, samp_rate = sampRate / 10, iq = True, subaudio = modulation_FSK_Sub_Audio_Bool, options="")
        self.satellites_fossasat_deframer_0 = satellites.components.deframers.fossasat_deframer(syncword_threshold = deframer_Syncword_Threshold_Int, options="")
        self.satellites_bpsk_demodulator_0 = satellites.components.demodulators.bpsk_demodulator(baudrate = modulation_Baud_Rate_Int, samp_rate = sampRate / 10, f_offset = freqOffset, differential = modulation_BPSK_Differential_Bool, manchester = modulation_BPSK_Manchester_Bool, iq = True, options="")
        self.satellites_ax25_deframer_1 = satellites.components.deframers.ax25_deframer(g3ruh_scrambler=deframer_G3RUH_Bool, options="")
        self.satellites_ax100_deframer_0_0 = satellites.components.deframers.ax100_deframer(mode = "ASM", scrambler = "CCSDS", syncword_threshold = deframer_Syncword_Threshold_Int, options="")
        self.satellites_ax100_deframer_0 = satellites.components.deframers.ax100_deframer(mode = "RS", scrambler = "CCSDS", syncword_threshold = deframer_Syncword_Threshold_Int, options="")
        self.satellites_ao40_fec_deframer_0 = satellites.components.deframers.ao40_fec_deframer(syncword_threshold = deframer_Syncword_Threshold_Int, short_frames = deframer_Short_Frames_Bool, crc = deframer_CRC16_Bool, options="")
        self.satellites_afsk_demodulator_0 = satellites.components.demodulators.afsk_demodulator(baudrate = modulation_Baud_Rate_Int, samp_rate = sampRate / 10, iq = True, af_carrier = modulation_AFSK_Audio_Carrier_Freq_Int, deviation = modulation_AFSK_Deviation_Int, options="")
        self.osmosdr_source_0 = osmosdr.source(
            args="numchan=" + str(1) + " " + ""
        )
        self.osmosdr_source_0.set_time_unknown_pps(osmosdr.time_spec_t())
        self.osmosdr_source_0.set_sample_rate(sampRate)
        self.osmosdr_source_0.set_center_freq(sdr_Freq_Int, 0)
        self.osmosdr_source_0.set_freq_corr(0, 0)
        self.osmosdr_source_0.set_dc_offset_mode(0, 0)
        self.osmosdr_source_0.set_iq_balance_mode(0, 0)
        self.osmosdr_source_0.set_gain_mode(False, 0)
        self.osmosdr_source_0.set_gain(sdr_LNA_Gain_Int, 0)
        self.osmosdr_source_0.set_if_gain(sdr_IF_Gain_Int, 0)
        self.osmosdr_source_0.set_bb_gain(sdr_BB_Gain_Int, 0)
        self.osmosdr_source_0.set_antenna('', 0)
        self.osmosdr_source_0.set_bandwidth(0, 0)
        self.low_pass_filter_1 = filter.fir_filter_ccf(
            10,
            firdes.low_pass(
                1,
                sampRate,
                main_LPF_Cutoff_Freq_Int,
                1000,
                firdes.WIN_HAMMING,
                6.76))
        self.dc_blocker_xx_0 = filter.dc_blocker_cc(32, True)
        self.blocks_wavfile_source_0 = blocks.wavfile_source('/home/stanislavb/satellite-recordings/gomx_1.wav', True)
        self.blocks_wavfile_sink_0 = blocks.wavfile_sink(fileNameRes, 1, int(sampRate / 10), 8)
        self.blocks_udp_sink_0 = blocks.udp_sink(gr.sizeof_char*1, main_Network_Stream_Address_Str, main_Network_Stream_Port_Int, 1472, True)
        self.blocks_selector_3 = blocks.selector(gr.sizeof_gr_complex*1,0,0)
        self.blocks_selector_3.set_enabled(True)
        self.blocks_selector_2 = blocks.selector(gr.sizeof_float*1,modulation_Type_Int,modulation_Deframer_Type_Int)
        self.blocks_selector_2.set_enabled(True)
        self.blocks_selector_1 = blocks.selector(gr.sizeof_gr_complex*1,0,0)
        self.blocks_selector_1.set_enabled(True)
        self.blocks_selector_0 = blocks.selector(gr.sizeof_gr_complex*1,0,modulation_Type_Int)
        self.blocks_selector_0.set_enabled(True)
        self.blocks_pdu_to_tagged_stream_0 = blocks.pdu_to_tagged_stream(blocks.byte_t, 'data')
        self.blocks_null_source_1 = blocks.null_source(gr.sizeof_float*1)
        self.blocks_message_debug_0 = blocks.message_debug()
        self.blocks_float_to_complex_0 = blocks.float_to_complex(1)
        self.blocks_file_sink_1 = blocks.file_sink(gr.sizeof_gr_complex*1, fileNameRaw, False)
        self.blocks_file_sink_1.set_unbuffered(False)
        self.blocks_file_sink_0 = blocks.file_sink(gr.sizeof_char*1, fileNameRes, False)
        self.blocks_file_sink_0.set_unbuffered(False)
        self.analog_wfm_rcv_0 = analog.wfm_rcv(
        	quad_rate=sampRate / 10,
        	audio_decimation=1,
        )
        self.analog_fm_deemph_0 = analog.fm_deemph(fs=sampRate / 10, tau=75e-6)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.satellites_ao40_fec_deframer_0, 'out'), (self.blocks_message_debug_0, 'print_pdu'))
        self.msg_connect((self.satellites_ao40_fec_deframer_0, 'out'), (self.blocks_pdu_to_tagged_stream_0, 'pdus'))
        self.msg_connect((self.satellites_ax100_deframer_0, 'out'), (self.blocks_message_debug_0, 'print_pdu'))
        self.msg_connect((self.satellites_ax100_deframer_0, 'out'), (self.blocks_pdu_to_tagged_stream_0, 'pdus'))
        self.msg_connect((self.satellites_ax100_deframer_0_0, 'out'), (self.blocks_message_debug_0, 'print_pdu'))
        self.msg_connect((self.satellites_ax100_deframer_0_0, 'out'), (self.blocks_pdu_to_tagged_stream_0, 'pdus'))
        self.msg_connect((self.satellites_ax25_deframer_1, 'out'), (self.blocks_message_debug_0, 'print_pdu'))
        self.msg_connect((self.satellites_ax25_deframer_1, 'out'), (self.blocks_pdu_to_tagged_stream_0, 'pdus'))
        self.msg_connect((self.satellites_fossasat_deframer_0, 'out'), (self.blocks_message_debug_0, 'print_pdu'))
        self.msg_connect((self.satellites_fossasat_deframer_0, 'out'), (self.blocks_pdu_to_tagged_stream_0, 'pdus'))
        self.msg_connect((self.satellites_u482c_deframer_0, 'out'), (self.blocks_message_debug_0, 'print_pdu'))
        self.msg_connect((self.satellites_u482c_deframer_0, 'out'), (self.blocks_pdu_to_tagged_stream_0, 'pdus'))
        self.connect((self.analog_fm_deemph_0, 0), (self.blocks_wavfile_sink_0, 0))
        self.connect((self.analog_wfm_rcv_0, 0), (self.blocks_selector_2, 3))
        self.connect((self.blocks_float_to_complex_0, 0), (self.blocks_selector_1, 1))
        self.connect((self.blocks_null_source_1, 0), (self.blocks_float_to_complex_0, 1))
        self.connect((self.blocks_pdu_to_tagged_stream_0, 0), (self.blocks_file_sink_0, 0))
        self.connect((self.blocks_pdu_to_tagged_stream_0, 0), (self.blocks_udp_sink_0, 0))
        self.connect((self.blocks_selector_0, 3), (self.analog_wfm_rcv_0, 0))
        self.connect((self.blocks_selector_0, 1), (self.satellites_afsk_demodulator_0, 0))
        self.connect((self.blocks_selector_0, 0), (self.satellites_bpsk_demodulator_0, 0))
        self.connect((self.blocks_selector_0, 2), (self.satellites_fsk_demodulator_0, 0))
        self.connect((self.blocks_selector_1, 0), (self.blocks_selector_0, 0))
        self.connect((self.blocks_selector_2, 6), (self.analog_fm_deemph_0, 0))
        self.connect((self.blocks_selector_2, 0), (self.satellites_ao40_fec_deframer_0, 0))
        self.connect((self.blocks_selector_2, 4), (self.satellites_ax100_deframer_0, 0))
        self.connect((self.blocks_selector_2, 5), (self.satellites_ax100_deframer_0_0, 0))
        self.connect((self.blocks_selector_2, 2), (self.satellites_ax25_deframer_1, 0))
        self.connect((self.blocks_selector_2, 3), (self.satellites_fossasat_deframer_0, 0))
        self.connect((self.blocks_selector_2, 1), (self.satellites_u482c_deframer_0, 0))
        self.connect((self.blocks_selector_3, 0), (self.low_pass_filter_1, 0))
        self.connect((self.blocks_wavfile_source_0, 0), (self.blocks_float_to_complex_0, 0))
        self.connect((self.dc_blocker_xx_0, 0), (self.blocks_selector_3, 1))
        self.connect((self.low_pass_filter_1, 0), (self.blocks_file_sink_1, 0))
        self.connect((self.low_pass_filter_1, 0), (self.blocks_selector_1, 0))
        self.connect((self.osmosdr_source_0, 0), (self.blocks_selector_3, 0))
        self.connect((self.osmosdr_source_0, 0), (self.dc_blocker_xx_0, 0))
        self.connect((self.satellites_afsk_demodulator_0, 0), (self.blocks_selector_2, 1))
        self.connect((self.satellites_bpsk_demodulator_0, 0), (self.blocks_selector_2, 0))
        self.connect((self.satellites_fsk_demodulator_0, 0), (self.blocks_selector_2, 2))


    def get_sampRate(self):
        return self.sampRate

    def set_sampRate(self, sampRate):
        self.sampRate = sampRate
        self.set_main_LPF_Cutoff_Freq_Int(self.sampRate - 10000)
        self.low_pass_filter_1.set_taps(firdes.low_pass(1, self.sampRate, self.main_LPF_Cutoff_Freq_Int, 1000, firdes.WIN_HAMMING, 6.76))
        self.osmosdr_source_0.set_sample_rate(self.sampRate)

    def get_main_Result_File_Prefix_Str(self):
        return self.main_Result_File_Prefix_Str

    def set_main_Result_File_Prefix_Str(self, main_Result_File_Prefix_Str):
        self.main_Result_File_Prefix_Str = main_Result_File_Prefix_Str
        self.set_fileNameRaw(self.main_Result_File_Prefix_Str + ".raw")
        self.set_fileNameRes(self.main_Result_File_Prefix_Str + ".dat")

    def get_sdr_LNA_Gain_Int(self):
        return self.sdr_LNA_Gain_Int

    def set_sdr_LNA_Gain_Int(self, sdr_LNA_Gain_Int):
        self.sdr_LNA_Gain_Int = sdr_LNA_Gain_Int
        self.osmosdr_source_0.set_gain(self.sdr_LNA_Gain_Int, 0)

    def get_sdr_IF_Gain_Int(self):
        return self.sdr_IF_Gain_Int

    def set_sdr_IF_Gain_Int(self, sdr_IF_Gain_Int):
        self.sdr_IF_Gain_Int = sdr_IF_Gain_Int
        self.osmosdr_source_0.set_if_gain(self.sdr_IF_Gain_Int, 0)

    def get_sdr_Freq_Int(self):
        return self.sdr_Freq_Int

    def set_sdr_Freq_Int(self, sdr_Freq_Int):
        self.sdr_Freq_Int = sdr_Freq_Int
        self.osmosdr_source_0.set_center_freq(self.sdr_Freq_Int, 0)

    def get_sdr_BB_Gain_Int(self):
        return self.sdr_BB_Gain_Int

    def set_sdr_BB_Gain_Int(self, sdr_BB_Gain_Int):
        self.sdr_BB_Gain_Int = sdr_BB_Gain_Int
        self.osmosdr_source_0.set_bb_gain(self.sdr_BB_Gain_Int, 0)

    def get_modulation_Type_Int(self):
        return self.modulation_Type_Int

    def set_modulation_Type_Int(self, modulation_Type_Int):
        self.modulation_Type_Int = modulation_Type_Int
        self.blocks_selector_0.set_output_index(self.modulation_Type_Int)
        self.blocks_selector_2.set_input_index(self.modulation_Type_Int)

    def get_modulation_FSK_Sub_Audio_Bool(self):
        return self.modulation_FSK_Sub_Audio_Bool

    def set_modulation_FSK_Sub_Audio_Bool(self, modulation_FSK_Sub_Audio_Bool):
        self.modulation_FSK_Sub_Audio_Bool = modulation_FSK_Sub_Audio_Bool

    def get_modulation_Deframer_Type_Int(self):
        return self.modulation_Deframer_Type_Int

    def set_modulation_Deframer_Type_Int(self, modulation_Deframer_Type_Int):
        self.modulation_Deframer_Type_Int = modulation_Deframer_Type_Int
        self.blocks_selector_2.set_output_index(self.modulation_Deframer_Type_Int)

    def get_modulation_Baud_Rate_Int(self):
        return self.modulation_Baud_Rate_Int

    def set_modulation_Baud_Rate_Int(self, modulation_Baud_Rate_Int):
        self.modulation_Baud_Rate_Int = modulation_Baud_Rate_Int

    def get_modulation_BPSK_Manchester_Bool(self):
        return self.modulation_BPSK_Manchester_Bool

    def set_modulation_BPSK_Manchester_Bool(self, modulation_BPSK_Manchester_Bool):
        self.modulation_BPSK_Manchester_Bool = modulation_BPSK_Manchester_Bool

    def get_modulation_BPSK_Differential_Bool(self):
        return self.modulation_BPSK_Differential_Bool

    def set_modulation_BPSK_Differential_Bool(self, modulation_BPSK_Differential_Bool):
        self.modulation_BPSK_Differential_Bool = modulation_BPSK_Differential_Bool

    def get_modulation_AFSK_Deviation_Int(self):
        return self.modulation_AFSK_Deviation_Int

    def set_modulation_AFSK_Deviation_Int(self, modulation_AFSK_Deviation_Int):
        self.modulation_AFSK_Deviation_Int = modulation_AFSK_Deviation_Int

    def get_modulation_AFSK_Audio_Carrier_Freq_Int(self):
        return self.modulation_AFSK_Audio_Carrier_Freq_Int

    def set_modulation_AFSK_Audio_Carrier_Freq_Int(self, modulation_AFSK_Audio_Carrier_Freq_Int):
        self.modulation_AFSK_Audio_Carrier_Freq_Int = modulation_AFSK_Audio_Carrier_Freq_Int

    def get_main_Network_Stream_Port_Int(self):
        return self.main_Network_Stream_Port_Int

    def set_main_Network_Stream_Port_Int(self, main_Network_Stream_Port_Int):
        self.main_Network_Stream_Port_Int = main_Network_Stream_Port_Int

    def get_main_Network_Stream_Address_Str(self):
        return self.main_Network_Stream_Address_Str

    def set_main_Network_Stream_Address_Str(self, main_Network_Stream_Address_Str):
        self.main_Network_Stream_Address_Str = main_Network_Stream_Address_Str

    def get_main_LPF_Cutoff_Freq_Int(self):
        return self.main_LPF_Cutoff_Freq_Int

    def set_main_LPF_Cutoff_Freq_Int(self, main_LPF_Cutoff_Freq_Int):
        self.main_LPF_Cutoff_Freq_Int = main_LPF_Cutoff_Freq_Int
        self.low_pass_filter_1.set_taps(firdes.low_pass(1, self.sampRate, self.main_LPF_Cutoff_Freq_Int, 1000, firdes.WIN_HAMMING, 6.76))

    def get_freqOffset(self):
        return self.freqOffset

    def set_freqOffset(self, freqOffset):
        self.freqOffset = freqOffset

    def get_fileNameRes(self):
        return self.fileNameRes

    def set_fileNameRes(self, fileNameRes):
        self.fileNameRes = fileNameRes
        self.blocks_file_sink_0.open(self.fileNameRes)
        self.blocks_wavfile_sink_0.open(self.fileNameRes)

    def get_fileNameRaw(self):
        return self.fileNameRaw

    def set_fileNameRaw(self, fileNameRaw):
        self.fileNameRaw = fileNameRaw
        self.blocks_file_sink_1.open(self.fileNameRaw)

    def get_deframer_Syncword_Threshold_Int(self):
        return self.deframer_Syncword_Threshold_Int

    def set_deframer_Syncword_Threshold_Int(self, deframer_Syncword_Threshold_Int):
        self.deframer_Syncword_Threshold_Int = deframer_Syncword_Threshold_Int

    def get_deframer_Short_Frames_Bool(self):
        return self.deframer_Short_Frames_Bool

    def set_deframer_Short_Frames_Bool(self, deframer_Short_Frames_Bool):
        self.deframer_Short_Frames_Bool = deframer_Short_Frames_Bool

    def get_deframer_G3RUH_Bool(self):
        return self.deframer_G3RUH_Bool

    def set_deframer_G3RUH_Bool(self, deframer_G3RUH_Bool):
        self.deframer_G3RUH_Bool = deframer_G3RUH_Bool

    def get_deframer_CRC16_Bool(self):
        return self.deframer_CRC16_Bool

    def set_deframer_CRC16_Bool(self, deframer_CRC16_Bool):
        self.deframer_CRC16_Bool = deframer_CRC16_Bool





def main(top_block_cls=fmDemod, options=None):
    tb = top_block_cls()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        sys.exit(0)

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    tb.start()

    while True:
        time.sleep(1)

    tb.stop()
    tb.wait()


if __name__ == '__main__':
    main()
