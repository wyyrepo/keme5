greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

RESOURCES = keme.qrc\
    keme-jsp.qrc \
    keme-jspa.qrc \
    keme-jspb.qrc \
    keme-jspc.qrc \
    keme-jspd.qrc \
    keme-jspe.qrc \
    keme-jspf.qrc

QT += sql
QT += xml
CODECFORTR = UTF-8
CONFIG += qt
RC_FILE = factur.rc
CONFIG += release

HEADERS += factur/main_factur.h \
           factur/listaseries.h \
           funciones.h \
           basedatos.h \
           aritmetica.h \
           factur/inicio.h \
           factur/inicial.h \
           creaempresa.h \
           conexion.h \
    factur/serie.h \
    factur/listadocs.h \
    factur/edita_doc.h \
    factur/listarefs.h \
    factur/edita_ref.h \
    tiposivacta.h \
    buscasubcuenta.h \
    factur/facturas.h \
    factur/factura.h \
    ret_irpf.h \
    ajuste_imagen_logo.h \
    subcuentas.h \
    ajuste_imagen.h \
    arbol.h \
    datos_accesorios.h \
    factur/traspasos.h \
    editarasiento.h \
    tabla_asientos.h \
    procesavencimiento.h \
    login.h \
    aux_express.h \
    factur/buscaref.h \
    asignainputdiario.h \
    buscaconcepto.h \
    conceptos.h \
    ivasoportado.h \
    ivarepercutido.h \
    aib.h \
    eib.h \
    introci.h \
    consmayor.h \
    ivadatosadic.h \
    ivasop_aut.h \
    ivarep_aut.h \
    exento.h \
    consultavencipase.h \
    pidenombre.h \
    retencion.h \
    cuadimprimayor.h \
    introfecha.h \
    nuevo_vencimiento.h \
    factur/lista_fact_predef.h \
    factur/lotes_fact.h \
    factur/lotes.h \
    acumuladosmes.h \
    asigna_fichdoc.h \
    nightcharts/nightcharts.h \
    factur/ed_cta_lote.h \
    vies_msj.h \
    soap/checkVatBinding.nsmap \
    soap/soapcheckVatBindingProxy.h \
    soap/soapH.h \
    soap/soapStub.h \
    soap/stdsoap2.h




SOURCES += factur/amain.cpp \
    factur/main_factur.cpp \
    factur/listaseries.cpp \
    funciones.cpp \
    basedatos.cpp \
    aritmetica.cpp \
    factur/inicio.cpp \
    factur/inicial.cpp \
    creaempresa.cpp \
    conexion.cpp \
    factur/serie.cpp \
    factur/listadocs.cpp \
    factur/edita_doc.cpp \
    factur/listarefs.cpp \
    factur/edita_ref.cpp \
    tiposivacta.cpp \
    buscasubcuenta.cpp \
    factur/facturas.cpp \
    factur/factura.cpp \
    ret_irpf.cpp \
    ajuste_imagen_logo.cpp \
    subcuentas.cpp \
    ajuste_imagen.cpp \
    arbol.cpp \
    datos_accesorios.cpp \
    factur/traspasos.cpp \
    editarasiento.cpp \
    tabla_asientos.cpp \
    procesavencimiento.cpp \
    login.cpp \
    aux_express.cpp \
    factur/buscaref.cpp \
    asignainputdiario.cpp \
    buscaconcepto.cpp \
    conceptos.cpp \
    ivasoportado.cpp \
    ivarepercutido.cpp \
    aib.cpp \
    eib.cpp \
    introci.cpp \
    consmayor.cpp \
    ivadatosadic.cpp \
    ivasop_aut.cpp \
    ivarep_aut.cpp \
    exento.cpp \
    consultavencipase.cpp \
    pidenombre.cpp \
    retencion.cpp \
    cuadimprimayor.cpp \
    introfecha.cpp \
    nuevo_vencimiento.cpp \
    factur/lista_fact_predef.cpp \
    factur/lotes_fact.cpp \
    factur/lotes.cpp \
    acumuladosmes.cpp \
    asigna_fichdoc.cpp \
    nightcharts/nightcharts.cpp \
    factur/ed_cta_lote.cpp \
    vies_msj.cpp \
    soap/soapC.cpp \
    soap/soapcheckVatBindingProxy.cpp \
    soap/stdsoap2.cpp


FORMS += factur/main_factur.ui \
         factur/listaseries.ui \
         factur/inicio.ui \
         factur/inicial.ui \
         creaempresa.ui \
         conexion.ui \
    factur/serie.ui \
    factur/listadocs.ui \
    factur/edita_doc.ui \
    factur/listarefs.ui \
    factur/edita_ref.ui \
    tiposivacta.ui \
    buscasubcuenta.ui \
    factur/facturas.ui \
    factur/factura.ui \
    ret_irpf.ui \
    ajuste_imagen_logo.ui \
    subcuentas.ui \
    ajuste_imagen.ui \
    datos_accesorios.ui \
    factur/traspasos.ui \
    tabla_asientos.ui \
    procesavencimiento.ui \
    login.ui \
    aux_express.ui \
    factur/buscaref.ui \
    asignainputdiario.ui \
    buscaconcepto.ui \
    conceptos.ui \
    ivasoportado.ui \
    ivarepercutido.ui \
    aib.ui \
    eib.ui \
    introci.ui \
    consmayor.ui \
    ivadatosadic.ui \
    ivasop_aut.ui \
    ivarep_aut.ui \
    exento.ui \
    consultavencipase.ui \
    pidenombre.ui \
    retencion.ui \
    cuadimprimayor.ui \
    introfecha.ui \
    nuevo_vencimiento.ui \
    factur/lista_fact_predef.ui \
    factur/lotes_fact.ui \
    factur/lotes.ui \
    acumuladosmes.ui \
    asigna_fichdoc.ui \
    factur/ed_cta_lote.ui \
    vies_msj.ui


unix { 
    UI_DIR = .ui
    MOC_DIR = .moc
    OBJECTS_DIR = .obj
}
