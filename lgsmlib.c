

#include <string.h>

#include "vmtel.h"
#include "vmlog.h"
#include "vmchset.h"
#include "vmsim.h"
#include "vmsms.h"

#include "lua.h"
#include "lauxlib.h"

enum VoiceCall_Status { IDLE_CALL, CALLING, RECEIVINGCALL, TALKING };

int g_gsm_incoming_call_cb_ref = LUA_NOREF;
int g_gsm_new_message_cb_ref = LUA_NOREF;

vm_ucm_id_info_struct g_uid_info;
int g_call_status = IDLE_CALL;
char g_incoming_number[42];

extern lua_State *L;

void call_listener_func(vm_call_listener_data *data) {
  if (data->type_op == VM_UCM_INCOMING_CALL_IND) {
    vm_ucm_incoming_call_ind_struct *ind =
        (vm_ucm_incoming_call_ind_struct *)data->data;
    g_uid_info.call_id = ind->uid_info.call_id;
    g_uid_info.group_id = ind->uid_info.group_id;
    g_uid_info.call_type = ind->uid_info.call_type;
    strcpy(g_incoming_number, (char *)ind->num_uri);
    g_call_status = RECEIVINGCALL;

    if (g_gsm_incoming_call_cb_ref != LUA_NOREF) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, g_gsm_incoming_call_cb_ref);
        lua_pushstring(L, g_incoming_number);
        lua_call(L, 1, 0);
    }
  } else if (data->type_op == VM_UCM_OUTGOING_CALL_IND) {
    vm_ucm_outgoing_call_ind_struct *ind =
        (vm_ucm_outgoing_call_ind_struct *)data->data;
    g_uid_info.call_id = ind->uid_info.call_id;
    g_uid_info.group_id = ind->uid_info.group_id;
    g_uid_info.call_type = ind->uid_info.call_type;
    strcpy(g_incoming_number, (char *)ind->num_uri);
    g_call_status = TALKING;
  } else if (data->type_op == VM_UCM_CONNECT_IND) {
    vm_ucm_connect_ind_struct *ind = (vm_ucm_connect_ind_struct *)data->data;
    g_uid_info.call_id = ind->uid_info.call_id;
    g_uid_info.group_id = ind->uid_info.group_id;
    g_uid_info.call_type = ind->uid_info.call_type;
    g_call_status = TALKING;
  } else if (data->type_op == VM_UCM_CALL_END) {
    g_call_status = IDLE_CALL;
  } else {
    vm_log_info("bad operation type");
  }
}

static void call_voiceCall_callback(vm_call_actions_callback_data *data) {
  if (data->type_act == VM_UCM_DIAL_ACT) {

    if (data->data_act_rsp.result_info.result == VM_UCM_RESULT_OK) {
      g_call_status = CALLING;
    } else {
      g_call_status = IDLE_CALL;
    }
  } else if (data->type_act == VM_UCM_ACCEPT_ACT) {
    if (data->data_act_rsp.result_info.result == VM_UCM_RESULT_OK) {
      g_call_status = TALKING;
    } else {
      g_call_status = IDLE_CALL;
    }
  } else if (data->type_act == VM_UCM_HOLD_ACT) {
    // not use
  } else if (data->type_act == VM_UCM_END_SINGLE_ACT) {
    g_call_status = IDLE_CALL;
  } else {
    // not use
  }
}

int _gsm_call(const char *phone_number) {
  vm_ucm_dial_act_req_struct req;
  vm_call_actions_data data;

  vm_ascii_to_ucs2((VMWSTR)req.num_uri, VM_UCM_MAX_NUM_URI_LEN,
                   (VMSTR)phone_number);
  req.call_type = VM_UCM_VOICE_CALL_TYPE;
  req.is_ip_dial = 0;
  req.module_id = 0;
  req.phb_data = NULL;

  data.type_act = VM_UCM_DIAL_ACT;
  data.data_act = (void *)&req;
  data.userdata = NULL;
  data.callback = call_voiceCall_callback;

  return vm_call_actions(&data);
}

int gsm_call(lua_State *L) {
  const char *phone_number = lua_tostring(L, -1);
  int result = _gsm_call(phone_number);
  lua_pushnumber(L, result);

  return 1;
}

int gsm_anwser(lua_State *L) {
  int result;
  vm_ucm_single_call_act_req_struct req;
  vm_call_actions_data data;

  req.action_uid.call_type = g_uid_info.call_type;
  req.action_uid.call_id = g_uid_info.call_id;
  req.action_uid.group_id = g_uid_info.group_id;

  data.type_act = VM_UCM_ACCEPT_ACT;
  data.data_act = (void *)&req;
  data.userdata = NULL;
  data.callback = call_voiceCall_callback;
  result = vm_call_actions(&data);

  lua_pushnumber(L, result);

  return 1;
}

int gsm_hang(lua_State *L) {
  int result = 0;
  vm_ucm_single_call_act_req_struct req;
  vm_call_actions_data data;

  // vm_log_info("callhangCall");

  if (IDLE_CALL != g_call_status) {
    req.action_uid.call_type = g_uid_info.call_type;
    req.action_uid.call_id = g_uid_info.call_id;
    req.action_uid.group_id = g_uid_info.group_id;

    data.type_act = VM_UCM_END_SINGLE_ACT;
    data.data_act = (void *)&req;
    data.userdata = NULL;
    data.callback = call_voiceCall_callback;
    result = vm_call_actions(&data);
  }

  lua_pushnumber(L, result);

  return 1;
}

int gsm_on_incoming_call(lua_State *L)
{
    lua_pushvalue(L, 1);
    g_gsm_incoming_call_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0;
}

/* The callback of sending SMS, for checking if an SMS is sent successfully. */
void _gsm_text_callback(int result){
  vm_log_debug("send sms callback, result = %d", result);
}

int gsm_text(lua_State *L)
{
    VMWCHAR number[42];
    VMWCHAR content[100];
    const char *phone_number = luaL_checkstring(L, 1);
    const char *message = luaL_checkstring(L, 2);

    vm_ascii_to_ucs2(content, 100*2, message);
	vm_ascii_to_ucs2(number, 42*2, phone_number);

    lua_pushnumber(L, vm_send_sms(number, content, _gsm_text_callback));

    return 1;
}

int _gsm_on_new_message(vm_sms_event_t* event_data){
    vm_sms_event_new_sms_t * event_new_message_ptr;
    vm_sms_new_msg_t * new_message_ptr = NULL;
    char content[100];
    /* Checks if this event is for new SMS message. */
    if(event_data->event_id == VM_EVT_ID_SMS_NEW_MSG){
        /* Gets the event info. */
        event_new_message_ptr = (vm_sms_event_new_sms_t *)event_data->event_info;

        /* Gets the message data. */
        new_message_ptr  =  event_new_message_ptr->msg_data;

        /* Converts the message content to ASCII. */
        vm_ucs2_to_ascii((VMSTR)content, 100, (VMWSTR)event_new_message_ptr->content);

        if (g_gsm_new_message_cb_ref != LUA_NOREF) {
            lua_rawgeti(L, LUA_REGISTRYINDEX, g_gsm_new_message_cb_ref);
            lua_pushstring(L, new_message_ptr->sc_number);
            lua_pushstring(L, content);
            lua_call(L, 2, 0);
        }

        return 1;
    }
    else{
        return 0;
    }
}

int gsm_on_new_message(lua_State *L)
{
    int ref;
    lua_pushvalue(L, 1);
    g_gsm_new_message_cb_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    lua_pushnumber(L, vm_sms_set_interrupt_event_handler(VM_EVT_ID_SMS_NEW_MSG, _gsm_on_new_message, L));

    return 1;
}

#undef MIN_OPT_LEVEL
#define MIN_OPT_LEVEL 0
#include "lrodefs.h"

const LUA_REG_TYPE gsm_map[] = {{LSTRKEY("call"), LFUNCVAL(gsm_call)},
                                {LSTRKEY("answer"), LFUNCVAL(gsm_anwser)},
                                {LSTRKEY("hang"), LFUNCVAL(gsm_hang)},
                                {LSTRKEY("on_incoming_call"), LFUNCVAL(gsm_on_incoming_call)},
                                {LSTRKEY("text"), LFUNCVAL(gsm_text)},
                                {LSTRKEY("on_new_message"), LFUNCVAL(gsm_on_new_message)},
                                {LNILKEY, LNILVAL}};

LUALIB_API int luaopen_gsm(lua_State *L) {
  vm_call_reg_listener(call_listener_func);

  luaL_register(L, "gsm", gsm_map);
  return 1;
}
