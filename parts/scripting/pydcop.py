import pydcopc


demarshallerdict = {}
marshallerdict = {}


def addMarshaller( type, marshal ):
    marshallerdict[type] = marshal


def addDemarshaller( type, marshal ):
    demarshallerdict[type] = marshal


def marshal_bool( arg ):
    if (type(arg) == type("")):
        arg = (arg == "true")
    return pydcopc.marshal_int8(arg)


def demarshal_bool( arg ):
    return pydcopc.demarshal_int8(arg)


def demarshal_void( data ):
    return (None, data)


#def marshal_DCOPRef( ref ):
#    res = marshal_QString(ref.app)
#    res = res + marshal_QString(ref.object)
#    res = res + marshal_QString(ref.type)


def demarshal_DCOPRef( data ):
    (appname, data) = pydcopc.demarshal_QCString(data)
    (objname, data) = pydcopc.demarshal_QCString(data)
    (typename, data) = pydcopc.demarshal_QCString(data)
    return (DCOPObject(appname, objname), data)


addMarshaller("QString", pydcopc.marshal_QString)
addMarshaller("QCString", pydcopc.marshal_QCString)
addMarshaller("int8", pydcopc.marshal_int8)
addMarshaller("int16", pydcopc.marshal_int16)
addMarshaller("int32", pydcopc.marshal_int32)
addMarshaller("int", pydcopc.marshal_int)
addMarshaller("uint8", pydcopc.marshal_uint8)
addMarshaller("uint16", pydcopc.marshal_uint16)
addMarshaller("uint32", pydcopc.marshal_uint32)
addMarshaller("uint", pydcopc.marshal_uint)
addMarshaller("float", pydcopc.marshal_float)
addMarshaller("double", pydcopc.marshal_double)
addMarshaller("bool", marshal_bool)

addDemarshaller("QString", pydcopc.demarshal_QString)
addDemarshaller("QCString", pydcopc.demarshal_QCString)
addDemarshaller("int8", pydcopc.demarshal_int8)
addDemarshaller("int16", pydcopc.demarshal_int16)
addDemarshaller("int32", pydcopc.demarshal_int32)
addDemarshaller("int", pydcopc.demarshal_int)
addDemarshaller("uint8", pydcopc.demarshal_uint8)
addDemarshaller("uint16", pydcopc.demarshal_uint16)
addDemarshaller("uint32", pydcopc.demarshal_uint32)
addDemarshaller("uint", pydcopc.demarshal_uint)
addDemarshaller("float", pydcopc.demarshal_float)
addDemarshaller("double", pydcopc.demarshal_double)
addDemarshaller("bool", demarshal_bool)
addDemarshaller("void", demarshal_void)
addDemarshaller("DCOPRef", demarshal_DCOPRef)


class Parameter:
    def __init__( self, type ):
        self.first = None
        self.second = None
        
        # Abbreviations commonly used
        if (type == "QStringList"):
            type = "QValueList<QString>"
        elif (type == "QCStringList"):
            type = "QValueList<QCString>"
            
        pos1 = type.find('<')
        if (pos1 == -1):
            self.type = type
            return

        pos2 = type.find('>', pos1)
        if (pos2 == -1):
            return

        self.type = type[0:pos1]

        pos3 = type.find(',', pos1+1)
        if (pos3 == -1):
            self.first = Parameter(type[pos1+1:pos2])
        else:
            self.first = Parameter(type[pos1+1:pos3])
            self.second = Parameter(type[pos3+1,pos2])


    def signature( self ):
        res = self.type
        if (self.first != None):
            res = res + "<" + self.first.signature()
            if (self.second != None):
                res = res + "," + self.second.signature()
            res = res + ">"
        return res


    def marshal( self, arg ):
        if (self.type == "QValueList"):
            if (type(arg) != type([])):
                raise TypeError, "expected list, got " + type(arg)
                return
            res = pydcopc.marshal_uint32(len(arg))
            for i in arg:
                res = res + self.first.marshal(i)
            return res
        elif (self.type == "QMap"):
            if (type(art) != type({})):
                raise TypeError, "expected dict, got " + type(arg)
                return
            res = pydcopc.marshal_uint32(len(arg))
            for i in arg.keys():
                res = res + self.first.marshal(i)
                res = res + self.second.marshal(arg[i])
            return res
        try:
            marshaller = marshallerdict[self.type]
        except KeyError:
            raise TypeError, "can not marshal " + self.type
        return marshaller(arg)


    def demarshal( self, data ):
        if (self.type == "QValueList"):
            (n, data) = pydcopc.demarshal_uint32(data)
            list = []
            for i in range(0,n):
                (element, data) = self.first.demarshal(data)
                list.append(element)
            return (list, data)
        elif (self.type == "QMap"):
            (n, data) = pydcopc.demarshal_uint32(data)
            dict = {}
            for i in range(0,n):
                (leftelement, data) = self.first.demarshal(data)
                (rightelement, data) = self.second.demarshal(data)
                dict[leftelement] = rightelement
            return (dict, data)
        try:
            demarshaller = demarshallerdict[self.type]
        except KeyError:
            raise TypeError, "can not demarshal " + self.type
        return demarshaller(data)


class Method:
    def __init__( self, signature ):
        self.type = None;

        pos1 = signature.find(' ')
        if (pos1 == -1):
            return
        self.type = Parameter(signature[0:pos1])

        pos2 = signature.find('(', pos1)
        if (pos2 == -1):
            return

        self.name = signature[pos1+1:pos2]

        pos3 = signature.find(')', pos2)
        if (pos3 == -1):
            return

        params = signature[pos2+1:pos3].strip() + ","
        self.parameters = []

        if params != ",":
            level = 0
            start = 0
            for i in range(0, len(params)):
                if (params[i] == ',' and level == 0):
                    space = params.find(' ', start)
                    if (space == -1 or space > i):
                        space = i
                    self.parameters.append(Parameter(params[start:space]))
                    start = i+1
                elif (params[i] == '<'):
                    level = level+1
                elif (params[i] == '>'):
                    level = level-1

        self.signature = self.name + "("
        for i in range(0, len(self.parameters)):
            if (i != 0):
                self.signature = self.signature + ","
            self.signature = self.signature + self.parameters[i].signature()
        self.signature = self.signature + ")"


    def marshal( self, *args ):
        j = 0
        data = ""
        for p in self.parameters:
            data = data + p.marshal(args[j])
            j = j+1
        return data


    def demarshal( self, replytype, replydata):
        (res, replydata) = self.type.demarshal(replydata)
        return res


class DCOPMethod:
        def __init__( self, appname, objname, method ):
                self.appname = appname
                self.objname = objname
                self.method = method
        def __repr__( self ):
            return "DCOPMethod(%s,%s,%s)" % ( self.appname, self.objname, self.method.signature() )
        def __str__( self ):
            return "DCOPMethod(%s,%s,%s)" % ( self.appname, self.objname, self.method.signature() )
        def __call__( self, *args ):
            parameters = self.method.parameters
            if (len(args) != len(parameters)):
                # Simulate exactly python's error messages
                if (len(args) > len(parameters)):
                    raise TypeError, "too many arguments; expected " + str(len(parameters)) \
                          + ", got " + str(len(args))
                else:
                    raise TypeError, "not enough arguments; expected " + str(len(parameters)) \
                          + ", got " + str(len(args))
            data = apply(self.method.marshal, args)
            # Distinguish calls from sends; only calls need demarshalling
            if (self.method.type == "ASYNC"):
                pydcopc.send(self.appname, self.objname, self.method.signature, data)
            else:
                (replytype, replydata) = pydcopc.call(self.appname, self.objname, self.method.signature, data)
                return self.method.demarshal(replytype, replydata)


class DCOPObject:
    def __init__( self, appname, objname ):
        self.appname = appname
        self.objname = objname
        self.methoddict = None

    def __repr__( self ):
        return "DCOPObject(%s,%s)" % ( self.appname, self.objname )
    def __str__( self ):
        return "DCOPObject(%s,%s)" % ( self.appname, self.objname )
    def __getattr__( self, item ):
        if item == "__repr__":
            return self.__repr__
        if item == "__str__":
            return self.__str__
        if item == "__call__":
            return self.__call__
        if self.methoddict == None:
            data = ""
            (replytype, replydata) = pydcopc.call(self.appname, self.objname, "functions()", data)
            if (replytype != "QCStringList"):
                raise RuntimeError, "functions() failed"
            (methodlist, replydata) = Parameter("QCStringList").demarshal(replydata)
            self.methoddict = {}
            for i in methodlist:
                m = Method(i)
                if (self.methoddict.has_key(m.name)):
                    raise RuntimeError, "DCOP interface is broken"
                self.methoddict[m.name] = m
        method = self.methoddict[item]
        return DCOPMethod( self.appname, self.objname, method )


def connect(sender, signal, func):
    pydcopc.connect(sender.appname, sender.objname, signal, func)
