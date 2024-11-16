/*
 * ssdp.c
 *
 *  Created on: Nov 15, 2024
 *      Author: kiro
 */


#include "ssdp.h"
#include "mongoose.h"
#include "mqtt_gen_strings.hpp"
#include "definitions.h"


#define ssdp_dev_name  				dev_common_name
#define ssdp_dev_manufacturer_name	dev_manufacturer_name
#define ssdp_dev_model_name			dev_model_name
#define ssdp_dev_ver				dev_sw_ver
#define ssdp_web_port				"40000"
#define mac_len						6

static const char ssdp_addr[] = "udp://239.255.255.250:1900";
char *dev_id = NULL;
char *buff = NULL;
static char * ip_addr_str = NULL; //Переменная для хранения айпи адреса
static struct mg_mgr *mgr = NULL;
struct mg_tcpip_if *mif = NULL;

// Статическое описание устройства для XML
static const char device_description [] =
    "<?xml version=\"1.0\"?>\n"
    "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\n"
		"<specVersion>\n"
			"<major>1</major>\n"
			"<minor>0</minor>\n"
		"</specVersion>\n"
		"<device>\n"
			"<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>\n"
			"<friendlyName>%s</friendlyName>\n"
			"<manufacturer>%s</manufacturer>\n"
			"<modelName>%s</modelName>\n"
			"<modelNumber>%s</modelNumber>\n"
			"<UDN>UUID:%s</UDN>\n"
			"<presentationURL>http://%s/index.html</presentationURL>\n"
		"</device>\n"
    "</root>\r\n";

// SSDP NOTIFY сообщение
static const char ssdp_notify_template[] =
    "NOTIFY * HTTP/1.1\r\n"
	"NTS: ssdp:alive\r\n"
    "HOST: 239.255.255.250:1900\r\n"
    "CACHE-CONTROL: max-age=1800\r\n"
    "SERVER: STM32 Mongoose/%s UPnP/2.0\r\n"
	"CONFIGID.UPNP.ORG:1\r\n"
    "LOCATION: http://%s:%s/device_description.xml\r\n"
    "NT:upnp:rootdevice\r\n"
    "USN:%s::upnp:rootdevice\r\n"
    "\r\n";



// Функция отправки SSDP NOTIFY
static void send_ssdp_notify(void *param) {

	struct mg_mgr *mgr = (struct mg_mgr *)param;
	struct mg_connection *udp_conn = mg_connect(mgr, ssdp_addr, NULL, NULL);

	size_t buff_len = (strlen(ssdp_notify_template) + IP_v6_STR_LEN  + strlen(ssdp_web_port) + 200);
	buff = calloc(buff_len, sizeof(char));  // +1 для '\0'
	if (!buff) return; //TODO add log
	mg_snprintf(ip_addr_str, sizeof(char) * IP_v6_STR_LEN, "%M", mg_print_ip, &udp_conn->loc);

	snprintf(buff, buff_len-1, ssdp_notify_template, MG_VERSION, ip_addr_str, ssdp_web_port, dev_id);

    if (udp_conn != NULL) {
        mg_send(udp_conn, buff, strlen(buff));
        mg_close_conn(udp_conn);
    }
    free(buff);
}

void ssdp_set_id(const uint8_t* id, uint8_t id_len){

	if (id && (id_len > 0)){
		if(dev_id) free(dev_id);
		dev_id = (char *) calloc(id_len * 2 + 1, sizeof(char));  // +1 для '\0'
		if (dev_id ){
	        // Проходим по каждому байту идентификатора
	        for (unsigned int i = 0; i < id_len; i++) {
	            // Конвертируем каждый байт в два символа и добавляем в строку
	            sprintf(&dev_id[i * 2], "%02X", id[i]);
	        }
		}else{
			return ;
		}
	}
}


static void dev_description_handler(struct mg_connection *c, struct mg_http_message *hm){
	if (mg_match(hm->method, mg_str("GET"), NULL)){
		mg_http_reply(c, 200, "", device_description, ssdp_dev_name, ssdp_dev_manufacturer_name, ssdp_dev_model_name, ssdp_dev_ver, dev_id, ip_addr_str);
	}else{
		mg_http_reply(c, 400, "", //TODO delete for release,
								"{\"status\":\"error\",\"message\":\"Unsupported method, support only GET and POST methods\"}\r\n");
	}

}


static void dev_404_handler(struct mg_connection *c, struct mg_http_message *hm){
	mg_http_reply(c, 404, "", //TODO delete for release,
							"{\"status\":\"error\",\"message\":\"Not found\"}\r\n");

}


static void ssdp_web_handler(struct mg_connection *c, int ev, void *ev_data){

	if (ev == MG_EV_HTTP_MSG) {
		struct mg_http_message *hm = (struct mg_http_message *) ev_data;

		if (mg_match(hm->uri, mg_str("/device_description.xml"), NULL)) {
			dev_description_handler(c, hm);
		}else if (mg_match(hm->uri, mg_str("/*"), NULL)) {
			dev_404_handler(c, hm);
		}
	}
}


void ssdp_start_server(void * param_mgr, void * param_mif){
	mgr 		= (struct mg_mgr *)param_mgr;
	mif			= (struct mg_tcpip_if *)param_mif;
	ip_addr_str = (char *) calloc(IP_v6_STR_LEN, sizeof(char));
	if (!ip_addr_str) return; //TODO add logging

	ssdp_set_id(mif->mac, mac_len);

	mg_timer_add(mgr, 30000, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, send_ssdp_notify, mgr);
	mg_http_listen(mgr, "http://0.0.0.0:40000", ssdp_web_handler, mif);
}

