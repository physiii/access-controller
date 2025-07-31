// services/network_manager.h

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

void network_manager_init(const char *device_id, const char *wifi_ssid, const char *wifi_password, const char *ap_ssid);
bool network_is_connected(void);

#endif // NETWORK_MANAGER_H 