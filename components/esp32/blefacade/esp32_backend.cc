#include "esp32_backend.hh"
#include "esp_bt.h"
#include "esp_log.h"
#include "esp_system.h"
#include <algorithm>

#include <blefacade/core/GATT/att_utils.hh>
#include <blefacade/core/GATT/service.hh>
#include <blefacade/core/GATT/uuid.hh>
#include <blefacade/core/GATT/uuid_defs.hh>
#include <blefacade/core/utils/utils.hh>

using namespace std::placeholders;
using namespace softeq::ble::core;
using namespace softeq::ble::esp;

#define GATTS_TAG "GATTS_DEMO"
#define TEST_DEVICE_NAME "ESP_GATTS_SOFTEQ_DEMO"

static uint8_t adv_config_done = 0;
#define adv_config_flag (1 << 0)
#define scan_rsp_config_flag (1 << 1)

//TODO : make a separate source file with utils

#define INDICATION_FLG (0x0002)
#define NOTIFY_FLG (0x0001)

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

bool ESP32Backend::init()
{
    esp_err_t ret;

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    ESP_LOGI(GATTS_TAG, "controller init started...");
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        goto bt_controller_deinit;
    }

    ESP_LOGI(GATTS_TAG, "controller enable...");
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        goto bt_controller_disable;
    }
    ESP_LOGI(GATTS_TAG, "bluedroid init...");
    ret = esp_bluedroid_init();
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        goto bluedroid_deinit;
    }
    ESP_LOGI(GATTS_TAG, "bluedroid enable...");
    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        goto bluedroid_disable;
    }

    ESP_LOGI(GATTS_TAG, "register callbacks...");

    // non-static methods callback registration magic. Adore its author! TODO: play with functional class members
    Callback<void(esp_gatts_cb_event_t, esp_gatt_if_t, esp_ble_gatts_cb_param_t *)>::func =
        std::bind(&ESP32Backend::gattsEventHandler, this, _1, _2, _3);

    ret = esp_ble_gatts_register_callback(static_cast<esp_gatts_cb_t>(
        Callback<void(esp_gatts_cb_event_t, esp_gatt_if_t, esp_ble_gatts_cb_param_t *)>::callback));
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "gatts register error, error code = %x", ret);
        goto bluedroid_disable;
    }

    Callback<void(esp_gap_ble_cb_event_t, esp_ble_gap_cb_param_t *)>::func =
        std::bind(&ESP32Backend::gapEventHandler, this, _1, _2);

    ret = esp_ble_gap_register_callback(
        static_cast<esp_gap_ble_cb_t>(Callback<void(esp_gap_ble_cb_event_t, esp_ble_gap_cb_param_t *)>::callback));
    if (ret)
    {
        ESP_LOGE(GATTS_TAG, "gap register error, error code = %x", ret);
        goto bluedroid_disable;
    }

    ESP_LOGI(GATTS_TAG, "Init is completed successfully\n");
    configured = true;
    return configured;
bluedroid_disable:
    (void)esp_bluedroid_disable();
bluedroid_deinit:
    (void)esp_bluedroid_deinit();
bt_controller_disable:
    (void)esp_bt_controller_disable();
bt_controller_deinit:
    (void)esp_bt_controller_deinit();
    return configured;
}

ESP32Backend::~ESP32Backend()
{
    if (configured)
    {
        (void)esp_bluedroid_disable();
        (void)esp_bluedroid_deinit();
        (void)esp_bt_controller_disable();
        (void)esp_bt_controller_deinit();
    }
}

bool ESP32Backend::configureProfile(Profile &profile)
{
    size_t idx = _profiles.size();
    _profiles.push_back(&profile);
    ESP_LOGI(GATTS_TAG, "Registered a profile with index: %d\n", idx);
    return esp_ble_gatts_app_register(idx);
}

#define CONFIG_SET_RAW_ADV_DATA
#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[] = {0x02, 0x01, 0x06, 0x02, 0x0a, 0xeb, 0x03, 0x03, 0xab, 0xcd};
static uint8_t raw_scan_rsp_data[] = {0x0f, 0x09, 0x45, 0x53, 0x50, 0x5f, 0x47, 0x41,
                                      0x54, 0x54, 0x53, 0x5f, 0x44, 0x45, 0x4d, 0x4f};
#endif

static uint8_t char1_str[] = "Descriptor";
static esp_attr_value_t gatts_demo_char1_val = {
    .attr_max_len = 0x40,
    .attr_len = sizeof(char1_str),
    .attr_value = char1_str,
};

bool ESP32Backend::indicate(Characteristic &chr, uint8_t *data, uint32_t len)
{
    unsigned int i = 3; //First profile fits 3rd interface in terms of ESP
    Descriptor *cccd = findCccd(chr);
    if (cccd && cccd->getData().len >= 2) //CCCD shall be tied to a 2-bytes variable at least
    {
        uint16_t cccdState = cccd->getData().data[0] | cccd->getData().data[1] << 8;
        if (cccdState == INDICATION_FLG || cccdState == NOTIFY_FLG)
        {
            for (const auto p : _profiles)
            {
                if (profileContainsAttribute(*p, chr))
                {
                    if (data)
                    {
                        esp_ble_gatts_send_indicate(
                            i, 0, chr.getHandle(), //TODO : find out if +1 is required for char value handle
                            len, data, cccdState == INDICATION_FLG);
                    }
                    else
                    {
                        return esp_ble_gatts_send_indicate(i, 0, chr.getHandle(), chr.getData().len,
                                                           reinterpret_cast<uint8_t *>(chr.getData().data),
                                                           (cccdState == INDICATION_FLG));
                    }
                }
                i++;
            }
        }
        else
        {
            ESP_LOGE(GATTS_TAG, "CCCD state does not allow to indicate/notify");
        }
    }
    else
    {
        // TODO : consider push indication and refuse of this check
        ESP_LOGE(GATTS_TAG, "Characteristic ddoesn't contain CCCD to notify/indicate");
    }
    return false;
}

static esp_gatt_perm_t getPerms(const AttFlags &permissions)
{
    esp_gatt_perm_t ret{};
    if (permissions.isSet(Permission::read))
    {
        ret |= ESP_GATT_PERM_READ;
    }
    if (permissions.isSet(Permission::write))
    {
        ret |= ESP_GATT_PERM_WRITE;
    }
    if (permissions.isSet(Permission::authSignedWr))
    {
        ret |= ESP_GATT_PERM_WRITE_AUTHORIZATION;
    }
    if (permissions.isSet(Permission::authSignedRd))
    {
        ret |= ESP_GATT_PERM_READ_AUTHORIZATION;
    }
    return ret;
}

static esp_gatt_char_prop_t getProps(const Characteristic &chr)
{
    esp_gatt_char_prop_t ret{};
    const AttFlags props = chr.getProperties();
    const AttFlags perms = chr.getPermissions();
    if (props.isSet(Property::broadcast))
    {
        ret |= ESP_GATT_CHAR_PROP_BIT_BROADCAST;
    }
    if (props.isSet(Property::writeWoResp))
    {
        ret |= ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
    }
    if (props.isSet(Property::notify))
    {
        ret |= ESP_GATT_CHAR_PROP_BIT_NOTIFY;
    }
    if (props.isSet(Property::indicate))
    {
        ret |= ESP_GATT_CHAR_PROP_BIT_INDICATE;
    }
    if (perms.isSet(Permission::read))
    {
        ret |= ESP_GATT_CHAR_PROP_BIT_READ;
    }
    if (perms.isSet(Permission::write))
    {
        ret |= ESP_GATT_CHAR_PROP_BIT_WRITE;
    }
    if (perms.isSet(Permission::authSignedWr) || perms.isSet(Permission::authSignedRd))
    {
        ret |= ESP_GATT_CHAR_PROP_BIT_AUTH;
    }
    return ret;
}

// TODO : use  namespace

void userUuidToEsp32(const UUID &from, esp_bt_uuid_t &to) // TODO : move  to class static members
{
    if (from.getLength() == UUID::Length::LEN16)
    {
        to.len = ESP_UUID_LEN_16;
        to.uuid.uuid16 = from.getValue16();
    }
    else if (from.getLength() == UUID::Length::LEN32)
    {
        to.len = ESP_UUID_LEN_32;
        to.uuid.uuid32 = from.getValue32();
    }
    else if (from.getLength() == UUID::Length::LEN128)
    {
        to.len = ESP_UUID_LEN_128;
        for (size_t i = 0; i < 128; ++i)
        {
            to.uuid.uuid128[i] = from.getValue128(i);
        }
    }
    else
    {
        assert(false); // Unknown UUID type
    }
}

Attribute *findByEspUuid(Profile &prof, esp_bt_uuid_t uuid)
{
    UUID ptrUuid;
    if (ESP_UUID_LEN_16 == uuid.len)
    {
        UUID usrUuid{uuid.uuid.uuid16, UUID::Length::LEN16};
        ptrUuid = std::move(usrUuid);
    }
    else if (ESP_UUID_LEN_32 == uuid.len)
    {
        UUID usrUuid{uuid.uuid.uuid32, UUID::Length::LEN32};
        ptrUuid = std::move(usrUuid);
    }
    else if (ESP_UUID_LEN_128 == uuid.len)
    {
        Uuid128 uuid128;
        std::copy_n(uuid.uuid.uuid128, uuid128.size(), uuid128.begin());
        UUID usrUuid{uuid128};
        ptrUuid = std::move(usrUuid);
    }
    else
    {
        return nullptr;
    }

    return findAttributeByUuid(prof, ptrUuid);
}

static unsigned getServiceHandlesNum(const Service &srv)
{
    unsigned res = 2; // 2 handles for just empty service
    for (size_t i = 0; i < srv.getCharNum(); ++i)
    {
        const Characteristic &chr = srv.getChar(i);
        for (size_t j = 0; j < chr.getDescrNum(); ++j)
        {
            res++; // Each descriptor takes just 1 handle
        }
        res += 2; // Each characteristic takes 2 handles
    }
    return res;
}

void ESP32Backend::gattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                     esp_ble_gatts_cb_param_t *param)
{
    unsigned profileIdx = gatts_if - 3; // skip 2 for generic services and 1 for GAP

    if (_profiles.size() <= static_cast<size_t>(profileIdx))
    {
        ESP_LOGE(GATTS_TAG, "Received an event for profile %d, but %d profiles are registered", profileIdx,
                 _profiles.size());
        assert(false);
    }
    Profile &_prof = *_profiles.get(profileIdx);

    switch (event)
    {
    case ESP_GATTS_REG_EVT: {
        if (param->reg.status == ESP_GATT_OK)
        {
            ESP_LOGI(GATTS_TAG, "REGISTER_APP_EVT, status %d, app_id %d\n", param->reg.status, param->reg.app_id);
        }
        else
        {
            ESP_LOGE(GATTS_TAG, "Reg app failed, app_id %04x, status %d\n", param->reg.app_id, param->reg.status);
            return;
        }

        esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(TEST_DEVICE_NAME);
        if (set_dev_name_ret)
        {
            ESP_LOGE(GATTS_TAG, "set device name failed, error code = %x", set_dev_name_ret);
        }
#ifdef CONFIG_SET_RAW_ADV_DATA
        esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
        if (raw_adv_ret)
        {
            ESP_LOGE(GATTS_TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
        }
        adv_config_done |= adv_config_flag;
        esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
        if (raw_scan_ret)
        {
            ESP_LOGE(GATTS_TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
        }
        adv_config_done |= scan_rsp_config_flag;
#endif // TODO : add another init method

        for (int i = 0; i < _prof.getServiceNum(); ++i)
        {
            ESP_LOGI(GATTS_TAG, "Service %d setting up...\n", i);
            Service &srv = _prof.getService(i);
            esp_gatt_srvc_id_t srv_id;
            srv_id.is_primary = srv.isPrimary();
            srv_id.id.inst_id = i;
            userUuidToEsp32(srv.getUuid(), srv_id.id.uuid);
            ESP_LOGI(GATTS_TAG, "Service %d uuid is %s \n", i, srv.getUuid().getPrintable().c_str());
            esp_ble_gatts_create_service(gatts_if, &srv_id, getServiceHandlesNum(srv));
        }

        break;
    }

    case ESP_GATTS_CREATE_EVT: // TODO : add errors warn messages
    {
        Service *srv = nullptr;
        if (ESP_GATT_OK == param->create.status)
        {
            esp_ble_gatts_start_service(param->create.service_handle);
            ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, status %d,  service_handle %d\n", param->create.status,
                     param->create.service_handle);
            ESP_LOGI(GATTS_TAG, "CREATE_SERVICE_EVT, inst_id %d", param->create.service_id.id.inst_id);

            srv = findServiceByUuid(_prof, param->create.service_id.id.uuid.uuid.uuid16);

            if (nullptr == srv)
            {
                ESP_LOGE(GATTS_TAG, "Did not found a service with the same inst_id: %d",
                         param->create.service_id.id.inst_id);
                break;
            }

            if (srv)
            {
                for (size_t i = 0; i < srv->getCharNum(); ++i)
                {
                    Characteristic &charc = srv->getChar(i);
                    esp_bt_uuid_t chrUuid;
                    userUuidToEsp32(charc.getUuid(), chrUuid);
                    esp_err_t add_char_ret =
                        esp_ble_gatts_add_char(srv->getHandle(), &chrUuid, getPerms(charc.getPermissions()),
                                               getProps(charc), &gatts_demo_char1_val, nullptr);
                    if (add_char_ret)
                    {
                        ESP_LOGE(GATTS_TAG, "add char failed, error code =%x", add_char_ret);
                        break;
                    }

                    for (size_t i = 0; i < charc.getDescrNum(); ++i)
                    {
                        const Descriptor &descr = charc.getDescr(i);
                        esp_bt_uuid_t descrUuid;
                        userUuidToEsp32(descr.getUuid(), descrUuid);

                        esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(
                            srv->getHandle(), &descrUuid, getPerms(descr.getPermissions()), nullptr, nullptr);
                        if (add_descr_ret)
                        {
                            ESP_LOGE(GATTS_TAG, "add descriptor failed, error code =%x", add_descr_ret);
                        }
                    }
                }
            }
        }
        else
        {
            ESP_LOGE(GATTS_TAG, "Service creation failed! Status %d,  service_handle %d\n", param->create.status,
                     param->create.service_handle);
            break;
        }
        break;
    }

    case ESP_GATTS_ADD_CHAR_EVT: {
        ESP_LOGI(GATTS_TAG, "ADD_CHAR_EVT, status %d, service_handle %d, attr_handle %d\n", param->add_char.status,
                 param->add_char.service_handle, param->add_char.attr_handle);

        Attribute *att = findByEspUuid(_prof, param->add_char.char_uuid);
        if (att)
        {
            att->setHandle(param->add_char.attr_handle);
        }
        else
        {
            ESP_LOGE(GATTS_TAG, "Did not found a descriptor with the same uuid: %x",
                     param->add_char.char_uuid.uuid.uuid16);
        }
        break;
    }

    case ESP_GATTS_ADD_CHAR_DESCR_EVT: {
        ESP_LOGI(GATTS_TAG, "ADD_CHAR_DESCR_EVT, status %d, service_handle %d, attr_handle %d\n",
                 param->add_char_descr.status, param->add_char_descr.service_handle, param->add_char_descr.attr_handle);

        Attribute *att = findByEspUuid(_prof, param->add_char_descr.descr_uuid);
        if (att)
        {
            att->setHandle(param->add_char_descr.attr_handle);
        }
        else
        {
            ESP_LOGE(GATTS_TAG, "Did not found a descriptor with the same uuid: %x",
                     param->add_char_descr.descr_uuid.uuid.uuid16);
        }
        break;
    }

    case ESP_GATTS_START_EVT: {
        ESP_LOGI(GATTS_TAG, "SERVICE_START_EVT, status %d, service_handle %d\n", param->start.status,
                 param->start.service_handle);
        break;
    }
    case ESP_GATTS_WRITE_EVT: {
        ESP_LOGW(GATTS_TAG, "GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d\n", param->write.conn_id,
                 param->write.trans_id, param->write.handle);

        esp_gatt_rsp_t response;
        memset(&response, 0, sizeof(esp_gatt_rsp_t));
        Attribute *attr = findAttributeByHandle(_prof, param->write.handle);
        if (attr)
        {
            WriteParam writeParam{};
            writeParam.conn_id = 0;
            memcpy(&(writeParam.conn_id), param->write.bda, ESP_BD_ADDR_LEN);
            ESP_LOGI(GATTS_TAG, "ESP_GATTS_WRITE_EVT, peer address = 0x%llx", writeParam.conn_id);
            Error result = attr->onWriteEvent(*this, param->write.value, param->write.len, writeParam);
            if (Error::ERR_OK == result)
            {
                response.attr_value.handle = param->write.handle;
                esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK,
                                            &response);
            }
            else
            {
                ESP_LOGI(GATTS_TAG, "Write process completed with code 0x%x", static_cast<uint8_t>(result));
                esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_ERROR,
                                            &response); //TODO : convert error from handler return
            }
            break;
        }
        else
        {
            ESP_LOGE(GATTS_TAG, "Did not found an attribute with the handle: %d", param->write.handle);
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_NOT_FOUND,
                                        &response);
            break;
        }
        break;
    }
    case ESP_GATTS_READ_EVT: {
        ESP_LOGW(GATTS_TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n, gatts_if %d", param->read.conn_id,
                 param->read.trans_id, param->read.handle, gatts_if);
        uint64_t address = 0;
        memcpy(&address, param->read.bda, ESP_BD_ADDR_LEN);
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_READ_EVT, peer address = 0x%llx", address);

        esp_gatt_rsp_t response;
        memset(&response, 0, sizeof(esp_gatt_rsp_t));
        Attribute *attr = findAttributeByHandle(_prof, param->read.handle);
        if (attr)
        {
            size_t len;
            ReadParam readParam{};
            readParam.conn_id = 0;
            memcpy(&readParam.conn_id, param->read.bda, ESP_BD_ADDR_LEN);
            Error result = attr->onReadEvent(*this, response.attr_value.value, len, readParam);
            if (Error::ERR_OK == result)
            {
                response.attr_value.len = len;
                response.attr_value.handle = param->read.handle;
                esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK,
                                            &response);
            }
            else
            {
                ESP_LOGI(GATTS_TAG, "Read process completed with code 0x%x", static_cast<uint8_t>(result));
                esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_ERROR,
                                            &response); //TODO : convert error from handler return
            }
            break;
        }
        else
        {
            ESP_LOGE(GATTS_TAG, "Did not found an attribute with the handle: %d", param->read.handle);
            esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_NOT_FOUND,
                                        &response);
            break;
        }
        break;
    }
    case ESP_GATTS_EXEC_WRITE_EVT: {
        EventParam eventParam{};
        for (auto evtHandler : extEventHandlers)
        {
            evtHandler(*this, Event::EXEC_WRITE, eventParam);
        }
        break;
    }
    case ESP_GATTS_DISCONNECT_EVT: {
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x", param->disconnect.reason);
        esp_ble_gap_start_advertising(&adv_params);
        if (!extEventHandlers.empty())
        {
            EventParam eventParam{};
            memcpy(&eventParam.conn_id, param->disconnect.remote_bda, ESP_BD_ADDR_LEN);
            for (auto evtHandler : extEventHandlers)
            {
                evtHandler(*this, Event::DISCONNECT, eventParam);
            }
        }
        break;
    }
    case ESP_GATTS_CONNECT_EVT: {
        uint64_t address = 0;
        memcpy(&address, param->connect.remote_bda, ESP_BD_ADDR_LEN);
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_CONNECT_EVT, peer address = 0x%llx", address);
        if (!extEventHandlers.empty())
        {
            EventParam eventParam{};
            eventParam.conn_id = address;
            for (auto evtHandler : extEventHandlers)
            {
                evtHandler(*this, Event::CONNECT, eventParam);
            }
        }
        break;
    }
    default:
        break;
    }
}

void ESP32Backend::gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
#ifdef CONFIG_SET_RAW_ADV_DATA
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
#else
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
#endif
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        // advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTS_TAG, "Advertising start failed\n");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTS_TAG, "Advertising stop failed\n");
        }
        else
        {
            ESP_LOGI(GATTS_TAG, "Stop adv successfully\n");
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(
            GATTS_TAG,
            "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
            param->update_conn_params.status, param->update_conn_params.min_int, param->update_conn_params.max_int,
            param->update_conn_params.conn_int, param->update_conn_params.latency, param->update_conn_params.timeout);
        break;
    default:
        break;
    }
}