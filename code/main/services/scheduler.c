char schedule_service_message[2000];
bool schedule_service_message_ready = false;

cJSON *schedule_ids = NULL;
cJSON *global_event_list[100];
int global_event_list_size = 0;
int cnt = 0;


void print_events() {
	printf("printing events...\n");
	for (int i = 0; i < global_event_list_size; i++) {
		printf("%s\n",cJSON_PrintUnformatted(global_event_list[i]));
	}
}

int check_schedule() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
  long int timezone_offset = central_time_hours*minutes_in_hour*seconds_in_minute;

  long int seconds_into_day = tv.tv_sec % 86400 + timezone_offset;
  long int minutes_into_day = seconds_into_day / seconds_in_minute;
  long int hours_into_day = minutes_into_day / minutes_in_hour;

  long int hours = hours_into_day;
  long int minutes = minutes_into_day % minutes_in_hour;
  long int seconds = seconds_into_day % seconds_in_minute;

  char id_str[10];
  char events_arr[100][20];
  int index = 0;

  cJSON *id =  NULL;
  for (int i = 0; i < global_event_list_size; i++)
  {
    cJSON *events = cJSON_GetObjectItemCaseSensitive(global_event_list[i],"event_list");
    cJSON *event = NULL;
    cJSON_ArrayForEach(event, events)
    {
			if (cJSON_GetObjectItemCaseSensitive(event,"event_id")) {
      	cJSON *id = cJSON_GetObjectItemCaseSensitive(event,"event_id");
      	char current_id[100];
      	sprintf(current_id,"%s",id->valuestring);
				int _seconds_into_day = cJSON_GetObjectItem(event,"seconds_into_day")->valueint;
	      int difference = _seconds_into_day - seconds_into_day;
	      printf("(%d)[%s] currently %lu, going off at %d\n",cnt++,current_id,seconds_into_day,_seconds_into_day);
	      if (difference == 0) {
	        cJSON *state = cJSON_GetObjectItem(event,"state");
	        int level = cJSON_GetObjectItem(state,"level")->valueint;
	        // set_brightness(level);
					dimmer_payload = cJSON_CreateObject();
		      cJSON *level_json = cJSON_CreateNumber(level);
		      cJSON_AddItemToObject(dimmer_payload, "level", level_json);
	        printf("Setting dimmer to %d\n",level);
	      }
			}
    }
  }

  //printf("time of day is %lu:%lu:%lu\n",hours,minutes,seconds);
  //printf("seconds into day %lu\n",seconds_into_day);
	return 1;
}

int add_service_id(char * id) {
  cJSON *service_ids = cJSON_CreateArray();
  cJSON *new_service_id =  cJSON_CreateString(id);
  cJSON_AddItemToArray(schedule_ids, new_service_id);
	printf("new scheduleids: %s\n",cJSON_PrintUnformatted(schedule_ids));
  char schedule_str[1000];
	cJSON * schedule_obj = cJSON_CreateObject();
	cJSON_AddItemToObject(schedule_obj,"service_ids",schedule_ids);
  strcpy(schedule_str,cJSON_PrintUnformatted(schedule_obj));
  printf("add_service_id %s\n",schedule_str);
  store_char("schedule_ids", schedule_str);
  return 0;
}

int create_schedule_ids() {
  cJSON *schedule = cJSON_CreateObject();
  schedule_ids = cJSON_CreateArray();
  cJSON_AddItemToObject(schedule, "service_ids", schedule_ids);
  char *schedule_str = cJSON_PrintUnformatted(schedule);
  store_char("schedule_ids",schedule_str);
  printf("create_schedule_ids\n");
  return 0;
}

int check_service_id(char * service_id) {
  if (schedule_ids == NULL) {
    create_schedule_ids();
  }
  bool id_found = false;
  cJSON *service_id_json =  NULL;
	printf("looping schedule_ids\n");
  cJSON_ArrayForEach(service_id_json, schedule_ids)
  {
    char current_id[15];
    sprintf(current_id,"%s",service_id_json->valuestring);
    if (strcmp(current_id,service_id)==0) {
      id_found = true;
    }
  }

  if (!id_found) {
    add_service_id(service_id);
  } else {
    printf("service id (%s) found in schedule ids.\n",service_id);
  }
	printf("check_service_id\n");
  return 0;
}

int store_event() {
  char service_id[250];
	char event_id[250];
	sprintf(event_id,"%s",cJSON_GetObjectItem(schedule_payload,"event_id")->valuestring);
  sprintf(service_id,"%s",cJSON_GetObjectItem(schedule_payload,"service_id")->valuestring);

  check_service_id(service_id);

	int index = 0;
	for (int i = 0; i < global_event_list_size; i++)
  {
		printf("global_event_list[i]: %s\n",cJSON_PrintUnformatted(global_event_list[i]));
		cJSON *events = cJSON_GetObjectItemCaseSensitive(global_event_list[i],"event_list");
    cJSON *event = NULL;
		int del_index = 0;
		bool del_index_found = false;
		char new_events_str[1000];
		cJSON *new_events = cJSON_CreateObject();

    cJSON_ArrayForEach(event, events)
    {
			printf("looping through events...\n");
			if (cJSON_GetObjectItemCaseSensitive(event,"service_id")) {
      	cJSON *service_id_obj = cJSON_GetObjectItemCaseSensitive(event,"service_id");
				cJSON *event_id_obj = cJSON_GetObjectItemCaseSensitive(event,"event_id");
      	char current_service_id[50];
				char current_event_id[50];
      	sprintf(current_service_id,"%s",service_id_obj->valuestring);
				sprintf(current_event_id,"%s",event_id_obj->valuestring);

				if (strcmp(current_service_id, service_id)==0) {
					if (strcmp(current_event_id, event_id)==0) {
							printf("deleting event to replace...%s %s",current_service_id,current_service_id);
							del_index = index;
							del_index_found = true;
					}
					cJSON_AddItemToArray(events,schedule_payload);

					cJSON_AddItemToObject(new_events, "event_list", events);
					strcpy(new_events_str,cJSON_PrintUnformatted(new_events));
					printf("storing new_event_list for %s...\n%s\n",current_service_id,new_events_str);
					store_char(service_id,new_events_str);
					global_event_list[i] = new_events;
					return 0;
				}
			}

			index++;
    }

		if (del_index_found) {
			printf("deleting old event at index %d",del_index);
			cJSON_DeleteItemFromArray(events,del_index);
			cJSON_AddItemToObject(new_events, "event_list", events);
			store_char(service_id,cJSON_PrintUnformatted(new_events));
			global_event_list[i] = events;
		}

		//if events for service id are empty, make it
		if (cJSON_GetArraySize(events)==0) {
			cJSON *new_event_arr = cJSON_CreateArray();
			cJSON_AddItemToArray(new_event_arr,schedule_payload);
			printf("schedule payload: %s\n",cJSON_PrintUnformatted(schedule_payload));
			cJSON *new_events = cJSON_CreateObject();
			cJSON_AddItemToObject(new_events, "event_list", new_event_arr);
			printf("storing new_events for %s...\n%s\n",service_id,cJSON_PrintUnformatted(new_events));
			store_char(service_id,cJSON_PrintUnformatted(new_events));
			global_event_list[i] = new_events;
		}
  }

	//if event list doesnt exist, make it
	if (global_event_list_size == 0) {
		cJSON *events = cJSON_CreateObject();
		cJSON *event_list = cJSON_CreateArray();
		cJSON_AddItemToArray(event_list,schedule_payload);
		cJSON_AddItemToObject(events,"event_list",event_list);
		global_event_list[0] = events;
		global_event_list_size++;
		store_char(service_id,cJSON_PrintUnformatted(events));
	}

	print_events();

  return 0;
}

int remove_event(char * service_id, char * event_id) {
	printf("removing event (%s) from service id (%s)\n",event_id,service_id);

	for (int i = 0; i < global_event_list_size; i++)
  {
    cJSON *events = cJSON_GetObjectItemCaseSensitive(global_event_list[i],"event_list");
		cJSON *new_events = cJSON_CreateArray();
		cJSON *new_events_obj = cJSON_CreateObject();
		cJSON *detached_item;
    cJSON *event = NULL;
		int index = 0;
		int del_index = 0;
		//printf("looping global event list for %s\n",service_id);
    cJSON_ArrayForEach(event, events)
    {
			if (cJSON_GetObjectItemCaseSensitive(event,"service_id")) {
				//printf("current event:\n%s\n",cJSON_PrintUnformatted(event));
      	cJSON *service_id_obj = cJSON_GetObjectItemCaseSensitive(event,"service_id");
				cJSON *event_id_obj = cJSON_GetObjectItemCaseSensitive(event,"event_id");
				char current_service_id[50];
				char current_event_id[50];

      	strcpy(current_service_id,service_id_obj->valuestring);
				strcpy(current_event_id,event_id_obj->valuestring);

				if (strcmp(current_service_id,service_id)==0) {
					if (strcmp(current_event_id,event_id)==0) { //delete event if found, otherwise add to new events
						printf("deleting event at index %d....%s\n",index,event_id);
						del_index = index;
						//detached_item = cJSON_DetachItemFromArray(events,index);
					} else {
						printf("adding event (%s) for %s\n",current_event_id,service_id);
						//cJSON_AddItemToArray(new_events,event);
					}
				}
			}
			index++;
    }

		cJSON_DeleteItemFromArray(events,del_index);
		cJSON_AddItemToObject(new_events_obj,"event_list",events);
		printf("new events \n%s\n",cJSON_PrintUnformatted(new_events_obj));

		printf("storing new events\n");
		store_char(service_id,cJSON_PrintUnformatted(new_events_obj));
		global_event_list[i] = new_events_obj;
  }
  return 0;
}

int handle_action(char * action) {
  printf("schedule action: %s\n",action);

	if (strcmp(action,"add")==0) {
    store_event();
	}

	if (strcmp(action,"remove")==0) {
		char service_id[250];
		char event_id[250];
	  sprintf(service_id,"%s",cJSON_GetObjectItem(schedule_payload,"service_id")->valuestring);
	  sprintf(event_id,"%s",cJSON_GetObjectItem(schedule_payload,"event_id")->valuestring);
		remove_event(service_id,event_id);
	}

	return 0;
}

int load_schedule_from_flash() {
  char * schedule_ids_str = get_char("schedule_ids");
  cJSON *schedule_ids_obj = cJSON_Parse(schedule_ids_str);
	cJSON *id =  NULL;
  cJSON *service_id =  NULL;
	int index = 0;

  if (cJSON_GetObjectItemCaseSensitive(schedule_ids_obj,"service_ids")) {
    schedule_ids = cJSON_GetObjectItemCaseSensitive(schedule_ids_obj,"service_ids");
    //lwsl_notice("loaded schedule ids...\n%s\n",schedule_ids_str);
  } else lwsl_err("no schedule found\n");


	cJSON_ArrayForEach(id, schedule_ids)
	{
		char id_str[15];
		sprintf(id_str,"%s",id->valuestring);
		char * event_list =	get_char(id_str);
		if (cJSON_Parse(event_list)) {
			cJSON * event_list_obj = cJSON_Parse(event_list);
			global_event_list[index] = event_list_obj;
			printf("loading event list:\n%\s\n",cJSON_PrintUnformatted(event_list_obj));
			index++;
		}
	}
	global_event_list_size = index;

  return 0;
}

static void schedule_service(void *pvParameter) {
  uint32_t io_num;
  int previous_state = 0;

  while (1) {
      //incoming messages from other services
      if (schedule_payload) {

        if (cJSON_GetObjectItem(schedule_payload,"action")) {
          char action[50];
          sprintf(action,"%s",cJSON_GetObjectItem(schedule_payload,"action")->valuestring);
          handle_action(action);
        }

        schedule_payload = NULL;
      }

      check_schedule();
      vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

int schedule_main() {
  printf("starting schedule service\n");
	load_schedule_from_flash();
  xTaskCreate(&schedule_service, "schedule_service_task", 10000, NULL, 5, NULL);
  return 0;
}
