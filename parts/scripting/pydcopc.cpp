#include "Python.h"
#include <qstring.h>
#include <qdatastream.h>
#include <dcopclient.h>
#include <dcopobject.h>


static PyObject *pydcopc_marshal_QString(PyObject */*self*/, PyObject *arg)
{
    QString str;
    char *p;
    Py_UNICODE *ustr; int ulen;

    if (PyArg_ParseTuple(arg, (char*)"s", &p)) {
        str = QString::fromLatin1(p);
    } else if (PyArg_ParseTuple(arg, (char*)"u#", &ustr, &ulen)) {
        str = QString((const QChar*)ustr, (uint)ulen);
    } else {
        PyErr_SetString(PyExc_TypeError, "Expected string or unicode string");
        return 0;
    }

    QByteArray data;
    QDataStream stream(data, IO_WriteOnly);
    stream << str;

    return Py_BuildValue((char*)"s#", data.data(), data.size());
}


static PyObject *pydcopc_marshal_QCString(PyObject */*self*/, PyObject *arg)
{
    QCString cstr;
    
    char *p;
    Py_UNICODE *ustr; int ulen;

    if (PyArg_ParseTuple(arg, (char*)"s", &p)) {
        cstr = QCString(p);
    } else if (PyArg_ParseTuple(arg, (char*)"u#", &ustr, &ulen)) {
        cstr = QString((const QChar*)ustr, (uint)ulen).latin1();
    } else {
        PyErr_SetString(PyExc_TypeError, "Expected string or unicode string");
        return 0;
    }

    QByteArray data;
    QDataStream stream(data, IO_WriteOnly);
    stream << cstr;

    return Py_BuildValue((char*)"s#", data.data(), data.size());
}


static PyObject *pydcopc_marshal_int8(PyObject */*self*/, PyObject *arg)
{
    int n;
    if (!PyArg_ParseTuple(arg, (char*)"i", &n))
        return 0;

    QByteArray data;
    QDataStream stream(data, IO_WriteOnly);
    stream << (Q_INT8) n;

    return Py_BuildValue((char*)"s#", data.data(), data.size());
}


static PyObject *pydcopc_marshal_int16(PyObject */*self*/, PyObject *arg)
{
    int n;
    if (!PyArg_ParseTuple(arg, (char*)"i", &n))
        return 0;

    QByteArray data;
    QDataStream stream(data, IO_WriteOnly);
    stream << (Q_INT16)n;

    return Py_BuildValue((char*)"s#", data.data(), data.size());
}


static PyObject *pydcopc_marshal_int32(PyObject */*self*/, PyObject *arg)
{
    long n;
    if (!PyArg_ParseTuple(arg, (char*)"l", &n))
        return 0;

    QByteArray data;
    QDataStream stream(data, IO_WriteOnly);
    stream << (Q_INT32)n;

    return Py_BuildValue((char*)"s#", data.data(), data.size());
}


// Methinks marshalling an int may be system-dependent?
static PyObject *pydcopc_marshal_int(PyObject */*self*/, PyObject *arg)
{
    long n;
    if (!PyArg_ParseTuple(arg, (char*)"l", &n))
        return 0;

    QByteArray data;
    QDataStream stream(data, IO_WriteOnly);
    stream << (int)n;

    return Py_BuildValue((char*)"s#", data.data(), data.size());
}


static PyObject *pydcopc_marshal_uint8(PyObject */*self*/, PyObject *arg)
{
    int n;
    if (!PyArg_ParseTuple(arg, (char*)"i", &n))
        return 0;

    QByteArray data;
    QDataStream stream(data, IO_WriteOnly);
    stream << (Q_INT8)n;

    return Py_BuildValue((char*)"s#", data.data(), data.size());
}


static PyObject *pydcopc_marshal_uint16(PyObject */*self*/, PyObject *arg)
{
    int n;
    if (!PyArg_ParseTuple(arg, (char*)"i", &n))
        return 0;

    QByteArray data;
    QDataStream stream(data, IO_WriteOnly);
    stream << (Q_UINT16)n;

    return Py_BuildValue((char*)"s#", data.data(), data.size());
}


static PyObject *pydcopc_marshal_uint32(PyObject */*self*/, PyObject *arg)
{
    long n;
    if (!PyArg_ParseTuple(arg, (char*)"l", &n))
        return 0;

    QByteArray data;
    QDataStream stream(data, IO_WriteOnly);
    stream << (Q_UINT32)n;

    return Py_BuildValue((char*)"s#", data.data(), data.size());
}


static PyObject *pydcopc_marshal_uint(PyObject */*self*/, PyObject *arg)
{
    long n;
    if (!PyArg_ParseTuple(arg, (char*)"l", &n))
        return 0;

    QByteArray data;
    QDataStream stream(data, IO_WriteOnly);
    stream << (uint)n;

    return Py_BuildValue((char*)"s#", data.data(), data.size());
}


static PyObject *pydcopc_marshal_float(PyObject */*self*/, PyObject *arg)
{
    double x;
    if (!PyArg_ParseTuple(arg, (char*)"d", &x))
        return 0;

    QByteArray data;
    QDataStream stream(data, IO_WriteOnly);
    stream << (float)x;

    return Py_BuildValue((char*)"s#", data.data(), data.size());
}


static PyObject *pydcopc_marshal_double(PyObject */*self*/, PyObject *arg)
{
    double x;
    if (!PyArg_ParseTuple(arg, (char*)"d", &x))
        return 0;

    QByteArray data;
    QDataStream stream(data, IO_WriteOnly);
    stream << (double)x;

    return Py_BuildValue((char*)"s#", data.data(), data.size());
}


static PyObject *pydcopc_demarshal_QString(PyObject */*self*/, PyObject *arg)
{
    char *datastr; int datalen;
    if (!PyArg_ParseTuple(arg, (char*)"s#", &datastr, &datalen))
        return 0;

    QByteArray data; data.duplicate(datastr, datalen);
    QDataStream stream(data, IO_ReadOnly);
    QString str;
    stream >> str;
    int advance = stream.device()->at();

    // The following simple form doesn't seem to work because of endianness?
    // return Py_BuildValue((char*)"(u#s#)", str.unicode(), str.length(), data.data()+advance, data.size()-advance);

    Py_UNICODE *p = new Py_UNICODE[str.length()];
    for (int i=0; i < (int)str.length(); ++i)
        p[i] = str.at(i).unicode();
    PyObject *obj = Py_BuildValue((char*)"(u#s#)", p, str.length(), data.data()+advance, data.size()-advance);
    delete p;
    return obj;
}


static PyObject *pydcopc_demarshal_QCString(PyObject */*self*/, PyObject *arg)
{
    char *datastr; int datalen;
    if (!PyArg_ParseTuple(arg, (char*)"s#", &datastr, &datalen))
        return 0;

    QByteArray data; data.duplicate(datastr, datalen);
    QDataStream stream(data, IO_ReadOnly);
    QCString str;
    stream >> str;
    int advance = stream.device()->at();

    return Py_BuildValue((char*)"(ss#)", str.data(), data.data()+advance, data.size()-advance);
}


static PyObject *pydcopc_demarshal_int8(PyObject */*self*/, PyObject *arg)
{
    char *datastr; int datalen;
    if (!PyArg_ParseTuple(arg, (char*)"s#", &datastr, &datalen))
        return 0;

    QByteArray data; data.duplicate(datastr, datalen);
    QDataStream stream(data, IO_ReadOnly);
    Q_INT8 n;
    stream >> n;
    int advance = stream.device()->at();

    return Py_BuildValue((char*)"(is#)", n, data.data()+advance, data.size()-advance);
}


static PyObject *pydcopc_demarshal_int16(PyObject */*self*/, PyObject *arg)
{
    char *datastr; int datalen;
    if (!PyArg_ParseTuple(arg, (char*)"s#", &datastr, &datalen))
        return 0;

    QByteArray data; data.duplicate(datastr, datalen);
    QDataStream stream(data, IO_ReadOnly);
    Q_INT16 n;
    stream >> n;
    int advance = stream.device()->at();

    return Py_BuildValue((char*)"(is#)", n, data.data()+advance, data.size()-advance);
}


static PyObject *pydcopc_demarshal_int32(PyObject */*self*/, PyObject *arg)
{
    char *datastr; int datalen;
    if (!PyArg_ParseTuple(arg, (char*)"s#", &datastr, &datalen))
        return 0;

    QByteArray data; data.duplicate(datastr, datalen);
    QDataStream stream(data, IO_ReadOnly);
    Q_INT32 n;
    stream >> n;
    int advance = stream.device()->at();

    return Py_BuildValue((char*)"(ls#)", (long)n, data.data()+advance, data.size()-advance);
}


static PyObject *pydcopc_demarshal_int(PyObject */*self*/, PyObject *arg)
{
    char *datastr; int datalen;
    if (!PyArg_ParseTuple(arg, (char*)"s#", &datastr, &datalen))
        return 0;

    QByteArray data; data.duplicate(datastr, datalen);
    QDataStream stream(data, IO_ReadOnly);
    uint n;
    stream >> n;
    int advance = stream.device()->at();

    return Py_BuildValue((char*)"(ls#)", (long)n, data.data()+advance, data.size()-advance);
}


static PyObject *pydcopc_demarshal_uint8(PyObject */*self*/, PyObject *arg)
{
    char *datastr; int datalen;
    if (!PyArg_ParseTuple(arg, (char*)"s#", &datastr, &datalen))
        return 0;

    QByteArray data; data.duplicate(datastr, datalen);
    QDataStream stream(data, IO_ReadOnly);
    Q_UINT8 n;
    stream >> n;
    int advance = stream.device()->at();

    return Py_BuildValue((char*)"(is#)", n, data.data()+advance, data.size()-advance);
}


static PyObject *pydcopc_demarshal_uint16(PyObject */*self*/, PyObject *arg)
{
    char *datastr; int datalen;
    if (!PyArg_ParseTuple(arg, (char*)"s#", &datastr, &datalen))
        return 0;

    QByteArray data; data.duplicate(datastr, datalen);
    QDataStream stream(data, IO_ReadOnly);
    Q_UINT16 n;
    stream >> n;
    int advance = stream.device()->at();

    return Py_BuildValue((char*)"(is#)", n, data.data()+advance, data.size()-advance);
}


static PyObject *pydcopc_demarshal_uint32(PyObject */*self*/, PyObject *arg)
{
    char *datastr; int datalen;
    if (!PyArg_ParseTuple(arg, (char*)"s#", &datastr, &datalen))
        return 0;

    QByteArray data; data.duplicate(datastr, datalen);
    QDataStream stream(data, IO_ReadOnly);
    Q_UINT32 n;
    stream >> n;
    int advance = stream.device()->at();

    return Py_BuildValue((char*)"(ls#)", (long)n, data.data()+advance, data.size()-advance);
}


static PyObject *pydcopc_demarshal_uint(PyObject */*self*/, PyObject *arg)
{
    char *datastr; int datalen;
    if (!PyArg_ParseTuple(arg, (char*)"s#", &datastr, &datalen))
        return 0;

    QByteArray data; data.duplicate(datastr, datalen);
    QDataStream stream(data, IO_ReadOnly);
    uint n;
    stream >> n;
    int advance = stream.device()->at();

    return Py_BuildValue((char*)"(ls#)", (long)n, data.data()+advance, data.size()-advance);
}


static PyObject *pydcopc_demarshal_float(PyObject */*self*/, PyObject *arg)
{
    char *datastr; int datalen;
    if (!PyArg_ParseTuple(arg, (char*)"s#", &datastr, &datalen))
        return 0;

    QByteArray data; data.duplicate(datastr, datalen);
    QDataStream stream(data, IO_ReadOnly);
    float x;
    stream >> x;
    int advance = stream.device()->at();

    return Py_BuildValue((char*)"(ds#)", (double)x, data.data()+advance, data.size()-advance);
}


static PyObject *pydcopc_demarshal_double(PyObject */*self*/, PyObject *arg)
{
    char *datastr; int datalen;
    if (!PyArg_ParseTuple(arg, (char*)"s#", &datastr, &datalen))
        return 0;

    QByteArray data; data.duplicate(datastr, datalen);
    QDataStream stream(data, IO_ReadOnly);
    double x;
    stream >> x;
    int advance = stream.device()->at();

    return Py_BuildValue((char*)"(ds#)", x, data.data()+advance, data.size()-advance);
}


static DCOPClient *client = 0;

static DCOPClient *dcopClient()
{
    if (!DCOPClient::mainClient()) {
        client = new DCOPClient;
        if (!client->attach()) {
            PyErr_SetString(PyExc_RuntimeError, "DCOP: could not attach");
            return NULL;
        }
    }
    return DCOPClient::mainClient();
}


// [replytype replydata] = pydcopc.call(appname, objname, signature, data)
static PyObject *pydcopc_call( PyObject */*self*/, PyObject *args )
{
    char *cappname;
    char *cobjname;
    char *csignature;
    char *datastr;
    int datalen;

    if (!PyArg_ParseTuple(args, (char*)"ssss#", &cappname, &cobjname, &csignature, &datastr, &datalen))
        return NULL;

    QCString appname(cappname);
    QCString objname(cobjname);
    QCString signature(csignature);
    QByteArray data; data.duplicate(datastr, datalen);

    DCOPClient *client = dcopClient();
    qDebug("Calling %s %s %s with datalen %d", appname.data(), objname.data(), signature.data(), data.size() );

    QCString replyType;
    QByteArray replyData;
    if (!client->call(appname, objname, signature, data, replyType, replyData)) {
        PyErr_SetString(PyExc_RuntimeError, "DCOP: call failed");
        return NULL;
    }

    return Py_BuildValue((char*)"(ss#)", replyType.data(), replyData.data(), replyData.size());
}


// pydcopc.send(appname, objname, signature, data)
static PyObject *pydcopc_send( PyObject */*self*/, PyObject *args )
{
    char *cappname;
    char *cobjname;
    char *csignature;
    char *datastr;
    int datalen;

    if (!PyArg_ParseTuple(args, (char*)"ssss#", &cappname, &cobjname, &csignature, &datastr, &datalen))
        return NULL;

    QCString appname(cappname);
    QCString objname(cobjname);
    QCString signature(csignature);
    QByteArray data; data.duplicate(datastr, datalen);

    DCOPClient *client = dcopClient();
    if (!client->send(appname, objname, signature, data)) {
        PyErr_SetString(PyExc_RuntimeError, "DCOP: send failed");
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


class PyDCOP_Dispatcher : public DCOPObject
{
public:
    PyDCOP_Dispatcher()
        : DCOPObject("PyDCOP_Dispatcher"), no(0)
    {}
    void connectToPython(QCString appname, QCString objname,
                         QCString signal, PyObject *func)
    {
        QCString slot;
        slot.setNum(++no);
        slot.prepend("f");
        slot.append("()");
        receivers[slot] = func;
        Py_INCREF(func);
        bool res = connectDCOPSignal(appname, objname, signal, slot, true);
        qDebug("PyDCOP connect %s/%s/%s/%s has result %s",
               appname.data(), objname.data(),
               signal.data(), slot.data(),
               (res? "true" : "false"));
    }
    virtual bool process(const QCString &fun, const QByteArray &data,
                         QCString& replyType, QByteArray &replyData)
    {
        qDebug("fun: %s", fun.data());
        QMap<QCString,PyObject*>::Iterator it = receivers.find(fun);
        if (it != receivers.end()) {
            Py_INCREF(Py_None);
            PyObject *arglist = Py_BuildValue((char*)"()");
            PyEval_CallObject(*it, arglist);
            return true;
        }
        return DCOPObject::process(fun, data, replyType, replyData);
    }
private:
    int no;
    QMap<QCString,PyObject*> receivers;
};


static PyDCOP_Dispatcher *dispatcher = 0;

PyDCOP_Dispatcher *signalDispatcher()
{
    if (!dispatcher)
        dispatcher = new PyDCOP_Dispatcher();
    return dispatcher;
}


// pydcopc.connect(sender, senderobj, signal, func)
static PyObject *pydcopc_connect( PyObject */*self*/, PyObject *args )
{
    char *appname, *objname, *signal;
    PyObject *func;
    
    if (!PyArg_ParseTuple(args, (char*)"sssO", &appname, &objname, &signal, &func))
        return 0;

    qDebug("Connecting %s/%s/%s", appname, objname, signal);
    signalDispatcher()->connectToPython(appname, objname, signal, func);

    Py_INCREF(Py_None);
    return Py_None;
}


static PyMethodDef pydcopc_methods[] = {
    	{ (char*) "call",               pydcopc_call,               METH_VARARGS, NULL },
    	{ (char*) "send",               pydcopc_send,               METH_VARARGS, NULL },
    	{ (char*) "connect",            pydcopc_connect,            METH_VARARGS, NULL },
	{ (char*) "marshal_QString",    pydcopc_marshal_QString,    METH_VARARGS, NULL },
	{ (char*) "marshal_QCString",   pydcopc_marshal_QCString,   METH_VARARGS, NULL },
	{ (char*) "marshal_int8",       pydcopc_marshal_int8,       METH_VARARGS, NULL },
	{ (char*) "marshal_int16",      pydcopc_marshal_int16,      METH_VARARGS, NULL },
	{ (char*) "marshal_int32",      pydcopc_marshal_int32,      METH_VARARGS, NULL },
	{ (char*) "marshal_int",        pydcopc_marshal_int,        METH_VARARGS, NULL },
	{ (char*) "marshal_uint8",      pydcopc_marshal_uint8,      METH_VARARGS, NULL },
	{ (char*) "marshal_uint16",     pydcopc_marshal_uint16,     METH_VARARGS, NULL },
	{ (char*) "marshal_uint32",     pydcopc_marshal_uint32,     METH_VARARGS, NULL },
	{ (char*) "marshal_uint",       pydcopc_marshal_uint,       METH_VARARGS, NULL },
	{ (char*) "marshal_float",      pydcopc_marshal_float,      METH_VARARGS, NULL },
	{ (char*) "marshal_double",     pydcopc_marshal_double,     METH_VARARGS, NULL },
	{ (char*) "demarshal_QString",  pydcopc_demarshal_QString,  METH_VARARGS, NULL },
	{ (char*) "demarshal_QCString", pydcopc_demarshal_QCString, METH_VARARGS, NULL },
	{ (char*) "demarshal_int8",     pydcopc_demarshal_int8,     METH_VARARGS, NULL },
	{ (char*) "demarshal_int16",    pydcopc_demarshal_int16,    METH_VARARGS, NULL },
	{ (char*) "demarshal_int32",    pydcopc_demarshal_int32,    METH_VARARGS, NULL },
	{ (char*) "demarshal_int",      pydcopc_demarshal_int,      METH_VARARGS, NULL },
	{ (char*) "demarshal_uint8",    pydcopc_demarshal_uint8,    METH_VARARGS, NULL },
	{ (char*) "demarshal_uint16",   pydcopc_demarshal_uint16,   METH_VARARGS, NULL },
	{ (char*) "demarshal_uint32",   pydcopc_demarshal_uint32,   METH_VARARGS, NULL },
	{ (char*) "demarshal_uint",     pydcopc_demarshal_uint,     METH_VARARGS, NULL },
	{ (char*) "demarshal_float",    pydcopc_demarshal_float,    METH_VARARGS, NULL },
	{ (char*) "demarshal_double",   pydcopc_demarshal_double,   METH_VARARGS, NULL },
        { NULL,                         NULL,                       0,            NULL }
};

extern "C"
{
    void initpydcopc()
    {
        (void) Py_InitModule((char*)"pydcopc", pydcopc_methods);
    }
}
