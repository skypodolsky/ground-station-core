#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>

#include "log.h"
#include "sdr.h"
#include "gnuradio.h"

int sdr_set_freq(double freq)
{
	xmlrpc_env env;
	xmlrpc_value * resultP;
	const char * const serverUrl = "http://" XMLRPC_DOPPLER_ADDR ":"XMLRPC_DOPPLER_PORT"/RPC2";
	const char * const methodName = XMLRPC_VAR_FREQ_CALL;

	xmlrpc_env_init(&env);

	xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, "Ground station control client", "1.0", NULL, 0);
	if (env.fault_occurred) {
		LOG_E("XMLRPC error (1): %s (%d)\n", env.fault_string, env.fault_code);
		xmlrpc_env_clean(&env);
		return -1;
	}

	resultP = xmlrpc_client_call(&env, serverUrl, methodName,
			"(i)", (xmlrpc_int32) freq);

	if (env.fault_occurred) {
		LOG_E("XMLRPC error (2): %s (%d)\n", env.fault_string, env.fault_code);
		goto exit;
	}

	xmlrpc_DECREF(resultP);

exit:
	xmlrpc_env_clean(&env);
	xmlrpc_client_cleanup();

	return 0;
}

int sdr_prepare_config(cfg_t *cfg, satellite_t *sat, const char *filename)
{
	char cfg_line[1024];
	LOG_I("config = %s\n", cfg->grc_config);
	FILE *fd = fopen(cfg->grc_config, "w");
	if (fd == NULL) {
		LOG_E("Couldn't open GNU Radio config file\n");
		return -1;
	}

	snprintf(cfg_line, sizeof(cfg_line), 	"[sdr]\n"
			"sdr_Freq=%d\n"
			"sdr_IF_Gain=%d\n"
			"sdr_BB_Gain=%d\n"
			"sdr_LNA_Gain=%d\n",
			sat->frequency,
			DEF_CONF_SDR_IF_GAIN,
			DEF_CONF_SDR_BB_GAIN,
			DEF_CONF_SDR_LNA_GAIN);

	fwrite(cfg_line, strlen(cfg_line), 1, fd);

	snprintf(cfg_line, sizeof(cfg_line), 	"[main]\n"
			"main_LPF_Cutoff_Freq=%d\n"
			"main_Network_Stream_Address=%s\n"
			"main_Network_Stream_Port=%s\n"
			"main_Result_File_Prefix=%s\n",
			sat->bandwidth,
			DEF_CONF_NETWORK_STREAM_ADDR,
			DEF_CONF_NETWORK_STREAM_PORT,
			filename);

	fwrite(cfg_line, strlen(cfg_line), 1, fd);

	snprintf(cfg_line, sizeof(cfg_line), 	"[modulation]\n"
			"modulation_Type=%d\n"
			"modulation_Baud_Rate=%d\n",
			sat->modulation,
			sat->baudRate);

	fwrite(cfg_line, strlen(cfg_line), 1, fd);

	switch (sat->modulation) {
		case MODULATION_BPSK:
			snprintf(cfg_line, sizeof(cfg_line), "modulation_BPSK_Manchester=%d\n"
					"modulation_BPSK_Differential=%d\n"
					"modulation_BPSK_Short_Preamb=%d\n"
					"modulation_BPSK_CRC16=%d\n",
					sat->bpskManchester,
					sat->bpskDifferential,
					sat->bpskShort,
					sat->bpskCRC16);

			fwrite(cfg_line, strlen(cfg_line), 1, fd);
			break;
		case MODULATION_AFSK:
			snprintf(cfg_line, sizeof(cfg_line), "modulation_AFSK_Audio_Carrier_Freq=%d\n"
					"modulation_AFSK_Deviation=%d\n"
					"modulation_G3RUH=%d\n",
					sat->afskAFC,
					sat->afskDeviation,
					sat->afskG3RUH);

			fwrite(cfg_line, strlen(cfg_line), 1, fd);
			break;
		case MODULATION_FSK:
			snprintf(cfg_line, sizeof(cfg_line), "modulation_FSK_Sub_Audio=%d\n"
					"modulation_G3RUH=%d\n",
					sat->fskSubAudio,
					sat->fskG3RUH);

			fwrite(cfg_line, strlen(cfg_line), 1, fd);
			break;
	};

	fclose(fd);
	return 0;
}


