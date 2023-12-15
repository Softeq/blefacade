#include <blefacade/security/softsecurity.hh>

using namespace softeq::ble::security;
using namespace softeq::ble::core;

void SoftSecurity::onClientDisconnect(uint64_t connectionID)
{
    authClients.erase(std::remove(authClients.begin(), authClients.end(), connectionID), authClients.end());
}

void SoftSecurity::authorize(uint64_t connectionID)
{
    if (!isAuthorized(connectionID))
    {
        authClients.push_back(connectionID);
    }
}

std::function<gattWriteHandler> SoftSecurity::protectCall(std::function<gattWriteHandler> hndlr)
{
    return [hndlr, this](BleBackendIf &bck, Attribute &att, const uint8_t *data, size_t len,
                         const WriteParam &param) -> ErrorCode {
        if (isAuthorized(param.conn_id))
        {
            return hndlr(bck, att, data, len, param);
        }
        else
        {
            return Error::ERR_NOT_PERMITTED;
        }
    };
}

std::function<gattReadHandler> SoftSecurity::protectCall(std::function<gattReadHandler> hndlr)
{
    return [hndlr, this](BleBackendIf &bck, Attribute &att, uint8_t *data, size_t &len,
                         const ReadParam &param) -> ErrorCode {
        if (isAuthorized(param.conn_id))
        {
            return hndlr(bck, att, data, len, param);
        }
        else
        {
            return Error::ERR_NOT_PERMITTED;
        }
    };
}

ErrorCode SoftSecurity::authHandler(softeq::ble::core::BleBackendIf &bck, Attribute &att, const uint8_t *data,
                                    size_t len, const WriteParam &param)
{
    (void)bck;
    (void)att;
    if (authenticate(data, len))
    {
        authorize(param.conn_id);
        return Error::ERR_OK;
    }
    return Error::ERR_AUTH;
}

ErrorCode SoftSecurity::readHandler(softeq::ble::core::BleBackendIf &bck, Attribute &att, uint8_t *data, size_t &len,
                                    const ReadParam &param)
{
    (void)bck;
    (void)att;
    (void)data;
    (void)param;
    len = 0;
    return Error::ERR_OK;
}

ErrorCode SoftSecurity::clientRegHandler(softeq::ble::core::BleBackendIf &bck, const Event &event,
                                         const EventParam &param)
{
    (void)bck;
    if (event == Event::DISCONNECT)
    {
        onClientDisconnect(param.conn_id);
    }
    return Error::ERR_OK;
}

void SoftSecurity::confAuthChar(Attribute &att, softeq::ble::core::BleBackendIf &bck)
{
    att.replaceReadHandler(bindReadCMethod(&softeq::ble::security::SoftSecurity::readHandler, this));
    att.replaceWriteHandler(bindWriteCMethod(&softeq::ble::security::SoftSecurity::authHandler, this));
    bck.addEventHandler(bindGattEventHandler(&softeq::ble::security::SoftSecurity::clientRegHandler, this));
    _authAtt = &att;
}

void SoftSecurity::protectAttribute(Attribute &att)
{
    if (_authAtt != &att) //To avoid protection of auth attribute
    {
        att.replaceReadHandler(this->protectCall(att.getOnReadHandler()));
        att.replaceWriteHandler(this->protectCall(att.getOnWriteHandler()));
    }
}

void SoftSecurity::protectService(Service &srv)
{
    for (size_t j = 0; j < srv.getCharNum(); j++)
    {

        Characteristic &chr = srv.getChar(j);
        protectAttribute(chr);
        for (size_t k = 0; k < chr.getDescrNum(); k++)
        {
            Descriptor &descr = chr.getDescr(k);
            protectAttribute(descr);
        }
    }
}

void SoftSecurity::protectProfile(Profile &prof)
{
    for (size_t i = 0; i < prof.getServiceNum(); i++)
    {
        protectService(prof.getService(i));
    }
}