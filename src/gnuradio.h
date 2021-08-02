#ifndef __GNURADIO_HEADER__
#define __GNURADIO_HEADER__

#define DEF_CONF_PYTHON					"python"
#define DEF_CONF_NETWORK_STREAM_ADDR 	"127.0.0.1"
#define DEF_CONF_NETWORK_STREAM_PORT 	"8080"
#define XMLRPC_DOPPLER_ADDR				"127.0.0.1"
#define XMLRPC_DOPPLER_PORT 			"7777"
#define XMLRPC_VAR_FREQ_CALL			"set_sdr_Freq_Int"

int sdr_set_freq(double freq);
int sdr_prepare_config(cfg_t *cfg, satellite_t *sat, const char *filename);

#endif /** __GNURADIO_HEADER__ */
