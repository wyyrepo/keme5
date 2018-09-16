/* ----------------------------------------------------------------------------------
    KEME-Contabilidad; aplicación para llevar contabilidades

    Copyright (C)

    Este programa es software libre: usted puede redistribuirlo y/o modificarlo 
    bajo los términos de la Licencia Pública General GNU publicada 
    por la Fundación para el Software Libre, ya sea la versión 3 
    de la Licencia, o (a su elección) cualquier versión posterior.

    Este programa se distribuye con la esperanza de que sea útil, pero 
    SIN GARANTÍA ALGUNA; ni siquiera la garantía implícita 
    MERCANTIL o de APTITUD PARA UN PROPÓSITO DETERMINADO. 
    Consulte los detalles de la Licencia Pública General GNU para obtener 
    una información más detallada. 

    Debería haber recibido una copia de la Licencia Pública General GNU 
    junto a este programa. 
    En caso contrario, consulte <http://www.gnu.org/licenses/>.
  ----------------------------------------------------------------------------------*/

#include "funciones.h"
#include "aritmetica.h"
#include "basedatos.h"
#include "privilegios.h"

#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QProcess>
#include <QProgressDialog>
#include <QtGui>
#include <Qt>
#include <QRegExp>
#include <QApplication>

#include "soap/soapcheckVatBindingProxy.h" // the proxy class, also #includes "soapH.h" and "soapStub.h"
#include "soap/checkVatBinding.nsmap"      // XML namespace mapping table (only needed once at the global level)


#ifdef Q_WS_MAC
   #include <CoreFoundation/CoreFoundation.h>
#endif

#if QT_VERSION >= 0x050000
 #define QT_WA(unicode, ansi) unicode
#endif

#ifdef _WIN64
   #define WINDOWS 1
   #include "qt_windows.h"
   #include "qwindowdefs_win.h"
   #include <shellapi.h>
#elif _WIN32
   #define WINDOWS 1
   #include "qt_windows.h"
   #include "qwindowdefs_win.h"
   #include <shellapi.h>
#else
   #define WINDOWS 0
#endif

#ifdef Q_WS_MAC
  #define OSX 1
#else
  #define OSX 0
#endif

#define COMPRONIVELSUP 0

#define LINEAS_FAC 10

bool privilegios[NUMPRIVILEGIOS];

bool copiardoc=true;

bool control_docs_repetidos=false;

QString convacoma(QString cad)
{
  QString origen;
  // if (origen[origen.length()-3]=='.') origen[origen.length()-3]=',';
  origen=cad.replace('.',',');
  return origen;
}


QString puntuanumerocomadec(QString origen)
{
 QString origen1=convacoma(origen).remove(' ');
 QString entera=origen1.left(uint(origen1.indexOf(',')));
 QString signo;
 // if (entera[1]=='-') {signo="-"; entera=entera.right(entera.length()-1);}
 if (entera.length()<4) return origen1;
 QString decimal=origen1.right(origen1.length()-uint(origen1.indexOf(',')));
 QString enterapuntos;
 int pasados=0;
 for (int veces=entera.length()-1;veces>=0;veces--)
     {
       pasados++;
       enterapuntos=enterapuntos.insert(0,entera[veces]);
       if (pasados==3 || pasados==6 || pasados==9)
           enterapuntos=enterapuntos.insert(0,'.');
     }
 if (enterapuntos[0]=='.') enterapuntos=enterapuntos.remove(0,1);
 if (enterapuntos[0]=='-' && enterapuntos[1]=='.') enterapuntos=enterapuntos.remove(1,1);
 QString resultado;
 resultado=signo;
 resultado+=enterapuntos;
 resultado+=decimal;
 return resultado;
}

 
QString puntuanumeropuntodec(QString origen)
{
 QString origen1=origen.remove(' ');
 QString entera=origen1.left(uint(origen1.indexOf('.')));
 QString signo;
 // if (entera[1]=='-') {signo="-"; entera=entera.right(entera.length()-1);}
 if (entera.length()<4) return origen1;
 QString decimal=origen1.right(origen1.length()-uint(origen1.indexOf('.')));
 QString enterapuntos;
 int pasados=0;
 for (int veces=entera.length()-1;veces>=0;veces--)
     {
       pasados++;
       enterapuntos=enterapuntos.insert(0,entera[veces]);
       if (pasados==3 || pasados==6 || pasados==9)
           enterapuntos=enterapuntos.insert(0,',');
     }
 if (enterapuntos[0]==',') enterapuntos=enterapuntos.remove(0,1);
 if (enterapuntos[0]=='-' && enterapuntos[1]==',') enterapuntos=enterapuntos.remove(1,1);
 QString resultado;
 resultado=signo;
 resultado+=enterapuntos;
 resultado+=decimal;
 return resultado;
}


QString ejerciciodelafecha(QDate fecha)
{
    return basedatos::instancia()->selectEjerciciodelafecha(fecha);
}

void calculaprimerasientoejercicio(QString ejercicio)
{
    basedatos::instancia()->calculaprimerasientoejercicio(ejercicio);
}


qlonglong primerasiento(QString ejercicio)
{
    return basedatos::instancia()->selectPrimerasiento(ejercicio);
}

QString numrelativa(QString numabsoluta)
{
    return basedatos::instancia()->numrelativa(numabsoluta);
}

qlonglong primasejerciciodelafecha(QDate fecha)
{
    return basedatos::instancia()->primasejerciciodelafecha(fecha);
}


QString trad_fecha()
{
  return QObject::tr("fecha");
}

QString trad_usuario()
{
  return QObject::tr("usuario");
}

QString trad_asiento()
{
  return QObject::tr("asiento");
}

QString trad_cuenta()
{
  return QObject::tr("cuenta");
}

QString trad_concepto()
{
  return QObject::tr("concepto");
}

QString trad_documento()
{
  return QObject::tr("documento");
}

QString trad_debe()
{
  return QObject::tr("debe");
}

QString trad_haber()
{
  return QObject::tr("haber");
}

QString trad_diario()
{
  return QObject::tr("diario");
}

QString trad_pase()
{
  return QObject::tr("pase");
}

QString trad_ejercicio()
{
  return QObject::tr("ejercicio");
}

QString trad_revisado()
{
  return QObject::tr("revisado");
}


QString filtro_a_lingu(QString fuente)
{
  QString destino=fuente;
  destino.replace(QString("fecha"),trad_fecha());
  destino.replace(QString("usuario"),trad_usuario());
  destino.replace(QString("asiento"),trad_asiento());
  destino.replace(QString("cuenta"),trad_cuenta());
  destino.replace(QString("concepto"),trad_concepto());
  destino.replace(QString("documento"),trad_documento());
  destino.replace(QString("debe"),trad_debe());
  destino.replace(QString("haber"),trad_haber());
  destino.replace(QString("diario"),trad_diario());
  destino.replace(QString("pase"),trad_pase());
  destino.replace(QString("ejercicio"),trad_ejercicio());
  destino.replace(QString("revisado"),trad_revisado());
  return destino;
}


QString lingu_a_filtro(QString fuente)
{
  QString destino=fuente;
  destino.replace(trad_fecha(),QString("fecha"));
  destino.replace(trad_usuario(),QString("usuario"));
  destino.replace(trad_asiento(),QString("asiento"));
  destino.replace(trad_cuenta(),QString("cuenta"));
  destino.replace(trad_concepto(),QString("concepto"));
  destino.replace(trad_documento(),QString("documento"));
  destino.replace(trad_debe(),QString("debe"));
  destino.replace(trad_haber(),QString("haber"));
  destino.replace(trad_diario(),QString("diario"));
  destino.replace(trad_pase(),QString("pase"));
  return destino;
}


QDate inicioejercicio(QString ejercicio)
{
    return basedatos::instancia()->selectAperturaejercicios(ejercicio);
}

QDate finejercicio(QString ejercicio)
{
    return basedatos::instancia()->selectCierreejercicios(ejercicio);
}

QString ultimonumasiento(QString filtro)
{
    return basedatos::instancia()->selectUltimonumasiento(filtro);
}

QString descripcioncuenta(QString qcodigo)
{
    return basedatos::instancia()->selectDescripcionplancontable(qcodigo);
}

double saldocuentaendiarioejercicio(QString cuenta,QString ejercicio)
{
    return basedatos::instancia()->selectSaldocuentaendiarioejercicio(cuenta,ejercicio);
}


QString nombreempresa()
{
    return basedatos::instancia()->selectEmpresaconfiguracion();
}

QString trayreport()
{
 QString ruta=trayconfig();
 ruta+=QDir::separator();
 ruta+="report";
 return ruta;
}

void verifica_dir_informes(QString pasa) // pasa es la ruta completa de .keme
{
    QString pasa00=pasa;
    pasa00+=QDir::separator();
    pasa00+="report";
    QDir d3(pasa00);
    if (!d3.exists()) d3.mkdir(pasa00);
    QString fich_mayor_jrxml=pasa00;
    fich_mayor_jrxml+=QDir::separator();
    fich_mayor_jrxml+="mayor.jrxml";
    if (eswindows()) fich_mayor_jrxml=QFile::encodeName(fich_mayor_jrxml);
    QFile mayor_informe(fich_mayor_jrxml);
    if (!mayor_informe.exists())
       {
         QString pasa22;
         if (eswindows()) pasa22=QFile::encodeName(traydoc());
            else pasa22=traydoc();
         pasa22+=QDir::separator();
         pasa22+="report";
         QDir dirdocreport(pasa22);
         QStringList filtersr;
         filtersr << "*";
         dirdocreport.setNameFilters(filtersr);
         //************falta copiado
         QStringList ficherosr=dirdocreport.entryList();
         for (int veces=0; veces<ficherosr.count(); veces++)
             {
              QString cad=traydoc();
              cad.append(QDir::separator());
              cad.append("report");
              cad.append(QDir::separator());
              // QString cfich=cfich.fromLocal8bit(ficheros.at(veces));
              QString cfich=ficherosr.at(veces);
              cad+=cfich;
              QString destino=pasa00;
              destino.append(QDir::separator());
              destino+=cfich;
              QString p;
              if (eswindows()) p=QFile::encodeName(cad);
                 else p=cad;
              QFile fichero(p);
              if (eswindows()) fichero.copy(QFile::encodeName(destino));
                 else fichero.copy(destino);
              // qWarning(destino.toAscii().constData());
              // QFile::copy ( cad, destino );
             }
       }
}

void copia_arch_config(QString origen, QString subdir="")
{
    QFile fichero(origen);

    QString directorio;
    if (WINDOWS) directorio=directorio.fromLocal8Bit(getenv("USERPROFILE"));
      else directorio=getenv("HOME");
    if (es_os_x()) directorio= getenv("HOME")+ directorio.fromLocal8Bit("/Library/Preferences");
    QString nombredir="/.keme";
    if (es_os_x()) nombredir="/keme5";
    nombredir[0]=QDir::separator();
    directorio+=nombredir;
    if (!subdir.isEmpty())
       {
         directorio+=QDir::separator();
         directorio+=subdir;
       }
    directorio+=QDir::separator();
    QString destino=directorio+origen.mid(origen.lastIndexOf("/")+1);
    fichero.copy(eswindows() ? QFile::encodeName(destino) : destino);
}

QString trayconfig(void)
{
    QString directorio;
    if (WINDOWS) directorio=directorio.fromLocal8Bit(getenv("USERPROFILE"));
      else directorio=getenv("HOME");
    if (es_os_x()) { directorio= getenv("HOME");
                     directorio+="/Library/Preferences";}
    QString nombredir="/.keme";
    if (es_os_x()) nombredir="/keme5";
     nombredir[0]=QDir::separator();
     directorio+=nombredir;
     QString pasa;
     if (eswindows()) pasa=QFile::encodeName(directorio);
        else pasa=directorio;
     QDir d2(pasa);
     if (!d2.exists())
       {
        d2.mkdir(pasa);
        d2.mkdir(pasa+QDir::separator()+"report");
        d2.mkdir(pasa+QDir::separator()+"report"+QDir::separator()+"jasp_xml2pdf_lib");
        // copiamos al directorio "pasa" los archivos del resource file
        copia_arch_config(":/estados/estados/balance_abreviado_plan_2007.est.xml");
        copia_arch_config(":/estados/estados/balance_plan_2007.est.xml");
        copia_arch_config(":/estados/estados/balance_pymes_IS2014.est.xml");
        copia_arch_config(":/estados/estados/balance_pymes_oficial.est.xml");
        copia_arch_config(":/estados/estados/cuenta_PyG_abreviada_plan_2007.est.xml");
        copia_arch_config(":/estados/estados/cuenta_PyG_plan_2007.est.xml");
        copia_arch_config(":/estados/estados/cuenta_pyg_pymes_IS2014.est.xml");
        copia_arch_config(":/estados/estados/cuenta_pyg_pymes_oficial.est.xml");
        // ------------------------------------------------------------
        // pasamos planes contables
        copia_arch_config(":/planes/planes/pgc_pymes_RD1515-2007.pln");
        copia_arch_config(":/planes/planes/pgc2007.pln");
        copia_arch_config(":/planes/planes/pla-pymes-2007-cat.pln");
        copia_arch_config(":/planes/planes/plan-entidades-sin-animo-lucro-simplificado2011.pln");
        copia_arch_config(":/planes/planes/plan-entidades-sin-animo-lucro2011.pln");

        // ------------------------------------------------------------
        // pasamos informes_jasp y java

        copia_arch_config(":/informes_jasp/informes_jasp/bcomp.jasper","report");

        copia_arch_config(":/informes_jasp/informes_jasp/bcomp.jrxml","report");
        copia_arch_config(":/informes_jasp/informes_jasp/bcompro-comp.jasper","report");
        copia_arch_config(":/informes_jasp/informes_jasp/bcompro-comp.jrxml","report");
        copia_arch_config(":/informes_jasp/informes_jasp/bss-comp.jasper","report");
        copia_arch_config(":/informes_jasp/informes_jasp/bss-comp.jrxml","report");
        copia_arch_config(":/informes_jasp/informes_jasp/bss.jasper","report");
        copia_arch_config(":/informes_jasp/informes_jasp/bss.jrxml","report");
        copia_arch_config(":/informes_jasp/informes_jasp/diario.jasper","report");
        copia_arch_config(":/informes_jasp/informes_jasp/diario.jrxml","report");
        copia_arch_config(":/informes_jasp/informes_jasp/estado_contable_1ejercicio.jasper","report");
        copia_arch_config(":/informes_jasp/informes_jasp/estado_contable_1ejercicio.jrxml","report");
        copia_arch_config(":/informes_jasp/informes_jasp/estado_contable_desglose.jasper","report");
        copia_arch_config(":/informes_jasp/informes_jasp/estado_contable_desglose.jrxml","report");
        copia_arch_config(":/informes_jasp/informes_jasp/estado_contable_no_colref.jasper","report");
        copia_arch_config(":/informes_jasp/informes_jasp/estado_contable_no_colref.jrxml","report");
        copia_arch_config(":/informes_jasp/informes_jasp/estado_contable_porcent_1col.jasper","report");
        copia_arch_config(":/informes_jasp/informes_jasp/estado_contable_porcent_1col.jrxml","report");
        copia_arch_config(":/informes_jasp/informes_jasp/estado_contable_porcent_2col.jasper","report");
        copia_arch_config(":/informes_jasp/informes_jasp/estado_contable_porcent_2col.jrxml","report");
        copia_arch_config(":/informes_jasp/informes_jasp/estado_contable.jasper","report");
        copia_arch_config(":/informes_jasp/informes_jasp/estado_contable.jrxml","report");
        copia_arch_config(":/informes_jasp/informes_jasp/factura_keme_retencion.jasper","report");
        copia_arch_config(":/informes_jasp/informes_jasp/factura_keme_retencion.jrxml","report");
        copia_arch_config(":/informes_jasp/informes_jasp/factura_keme.jasper","report");
        copia_arch_config(":/informes_jasp/informes_jasp/factura_keme.jrxml","report");
        copia_arch_config(":/informes_jasp/informes_jasp/mayor.jasper","report");
        copia_arch_config(":/informes_jasp/informes_jasp/mayor.jrxml","report");

        copia_arch_config(":/java/java/jasp_xml2pdf.jar","report");

        copia_arch_config(":/java/java/jasp_xml2pdf_lib/barbecue-1.5-beta1.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/barcode4j-2.1.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/commons-beanutils-1.8.0.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/commons-collections-2.1.1.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/commons-digester-2.1.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/commons-javaflow-20060411.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/commons-logging-1.1.1.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/groovy-all-2.0.1.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/iText-2.1.7.js2.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/jasperreports-5.5.0.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/jasperreports-applet-5.5.0.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/jasperreports-fonts-5.5.0.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/jasperreports-javaflow-5.5.0.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/jcommon-1.0.15.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/jdt-compiler-3.1.1.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/jfreechart-1.0.12.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/log4j-1.2.15.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/mondrian-3.1.1.12687.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/png-encoder-1.5.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/poi-3.7-20101029.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/rhino-1.7R3.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/serializer.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/servlet-api-2.4.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/xalan-2.7.1.jar","report/jasp_xml2pdf_lib");
        copia_arch_config(":/java/java/jasp_xml2pdf_lib/xml-apis.jar","report/jasp_xml2pdf_lib");


        // ------------------------------------------------------------
        /*
        QString pasa2;
        if (eswindows()) pasa2=QFile::encodeName(traydoc());
           else pasa2=traydoc();
        QDir dirdoc(pasa2);
        QStringList filters;
        filters << "*.xml" << "*.pln";
        dirdoc.setNameFilters(filters);
        QStringList ficheros=dirdoc.entryList();
        for (int veces=0; veces<ficheros.count(); veces++)
            {
             QString cad=traydoc();
             cad.append(QDir::separator());
             // QString cfich=cfich.fromLocal8bit(ficheros.at(veces));
             QString cfich=ficheros.at(veces);
             cad+=cfich;
             QString destino=directorio;
             destino.append(QDir::separator());
             destino+=cfich;
             QString p;
             if (eswindows()) p=QFile::encodeName(cad);
                else p=cad;
             QFile fichero(p);
             if (eswindows()) fichero.copy(QFile::encodeName(destino));
                else fichero.copy(destino);
             // qWarning(destino.toAscii().constData());
             // QFile::copy ( cad, destino );
            }
        if (es_os_x()) return directorio;
        QObject *parent=NULL;
        QProcess *myProcess = new QProcess(parent);
        if (!WINDOWS)
         {
          QDir directorio;
          QString destino=getenv("HOME");
          destino+="/Desktop";
          if (!directorio.exists(destino))
             {
              destino=getenv("HOME");
              destino+="/Escritorio";
             }
          if (directorio.exists ( destino ) )
             {
              #ifdef SMART
              QString cadexec="cp /usr/share/applications/ContaSmart-Keme.desktop ";
              #else
              QString cadexec="cp /usr/share/applications/KEME-Contabilidad.desktop ";
              #endif
              cadexec+=destino;
              myProcess->start(cadexec);
              myProcess->waitForFinished ();
             }
         }
        delete myProcess;
        */
       }

     // verifica_dir_informes(pasa);

     return directorio;
}


int anchocuentas()
{
    return basedatos::instancia()->selectAnchocuentasconfiguracion();
}

bool sobreescribeproxasiento(QString ejercicio)
{
    return basedatos::instancia()->sobreescribeproxasiento(ejercicio);
}

QString convapunto(QString cadena)
{
  return cadena.replace(',','.');
}


QString quitacomillas(QString cadena)
{
  QString resultado;
  resultado=cadena.remove(34); // comillas dobles
  resultado=resultado.remove(39); // comillas simples
  return resultado;
}


bool fechacorrespondeaejercicio(QDate fecha)
{
 if (ejerciciodelafecha(fecha).length()==0) return false;
 return true;
}

bool fechadejerciciocerrado(QDate fecha)
{
  QString ejercicio;
  ejercicio=ejerciciodelafecha(fecha);
  if (ejerciciocerrado(ejercicio)) return true;
  return false;
}

int existecodigoplan(QString cadena,QString *qdescrip)
{
    return basedatos::instancia()->existecodigoplan(cadena,qdescrip);
}


int existesubcuenta(QString codigo)
{

 QString ladescrip="";
 if (!cod_longitud_variable())
   {
    if (existecodigoplan(codigo,&ladescrip) && codigo.length()==anchocuentas()) return 1;
     else return 0;
   }
   else
       {
        if (existecodigoplan(codigo,&ladescrip) && esauxiliar(codigo)) return 1;
         else return 0;
       }
 return 0;
}


int escuentadeivasoportado(const QString subcuenta)
{
    return basedatos::instancia()->escuentadeivasoportado(subcuenta);
}

int escuentadeivarepercutido(const QString subcuenta)
{
    return basedatos::instancia()->escuentadeivarepercutido(subcuenta);
}

int ejerciciocerrado(QString ejercicio)
{
    return basedatos::instancia()->selectCerradoejercicios(ejercicio);
}

void borraasientos(QString inicial,QString final,QString ejercicio)
{
    basedatos::instancia()->borraasientos(inicial,final,ejercicio);
}

bool amortenintasientos(QString inicial,QString final, QString ejercicio)
{
    return basedatos::instancia()->amortenintasientos(inicial,final,ejercicio);
}


bool paseenasiento(QString pase, QString asiento)
{
    return basedatos::instancia()->paseenasiento(pase,asiento);
}


int subgrupocongrupo(QString cadena)
{
    return basedatos::instancia()->subgrupocongrupo(cadena);
}


int cuentaconsubgrupo(QString cadena)
{
    return basedatos::instancia()->cuentaconsubgrupo(cadena);
}


int codigoconcuenta3digitos(QString cadena)
{
    return basedatos::instancia()->codigoconcuenta3digitos(cadena);
}

void insertaenplan(QString codigo,QString descripcion)
{
    QString cadena;
    if (!existecodigoplan(codigo,&cadena))
    {
        basedatos::instancia()->insertPlancontable(codigo,descripcion,false);
    }
}


void inserta_auxiliar_enplan(QString codigo,QString descripcion)
{
    QString cadena;
    if (!existecodigoplan(codigo,&cadena))
    {
        basedatos::instancia()->insertPlancontable(codigo,descripcion,true);
    }
}

void guardacambiosplan(QString codigo,QString descripcion)
{
    basedatos::instancia()->updatePlancontabledescripcioncodigo(codigo,descripcion);
}

int itemplaneliminado(QString qcodigo)
{
    return basedatos::instancia()->itemplaneliminado(qcodigo);
}

void eliminasubcuenta(QString qcodigo)
{
    basedatos::instancia()->deleteDatossubcuentacuenta(qcodigo);
    basedatos::instancia()->deleteSaldossubcuentacodigo(qcodigo);
    basedatos::instancia()->deletePlancontablecodigo(qcodigo);
}


int cuentaendiario(QString cadena)
{
    return basedatos::instancia()->cuentaendiario(cadena);
}

QString expandepunto(QString cadena,int anchocad)
{
  if (anchocad==0) return cadena;
  int indice=0,longitud=0,veces;
  if (!cadena.contains('.')) return cadena;
  while (cadena.contains('.'))
    {
     indice=cadena.indexOf('.');
     cadena.remove(indice,1);
    }
  longitud=cadena.length();
  for (veces=0;veces<anchocad-longitud;veces++)
       cadena.insert(indice,'0');
  return cadena;
}


int existendatosaccesorios(QString cuenta, QString *razon, QString *ncomercial, QString *cif,
            QString *nifrprlegal, QString *domicilio, QString *poblacion,
            QString *codigopostal, QString *provincia, QString *pais,
            QString *tfno, QString *fax, QString *email, QString *observaciones,
            QString *ccc, QString *cuota, bool *venciasoc, QString *codvenci,
            QString *tesoreria, bool *ivaasoc, QString *cuentaiva, QString *cta_base_iva, QString *tipoiva,
            QString *conceptodiario, QString *web, QString *claveidfiscal, bool *procvto,
            QString *paisdat, QString *tipo_ret, QString *tipo_operacion_ret,
            bool *ret_arrendamiento, int *vfijo, QString *cuenta_ret_asig,
            bool *es_ret_asig_arrend, QString *tipo_ret_asig, QString *tipo_oper_ret_asig,
            bool *dom_bancaria, QString *IBAN, QString *nriesgo, QString *cedente1,
            QString *cedente2, QString *bic, QString *sufijo,
            QString *ref_mandato, QDate *firma_mandato,
            bool *caja_iva,
            QString *imagen)
{
    QSqlQuery query = basedatos::instancia()->selectTodoDatossubcuentacuenta(cuenta);
    // pais_dat, tipo_ret, tipo_operacion_ret, ret_arrendamiento
    if ( (query.isActive()) && (query.first()) )
    {
        *razon = query.value(1).toString();
        *ncomercial = query.value(2).toString();
        *cif = query.value(3).toString();
        *nifrprlegal = query.value(4).toString();
        *domicilio = query.value(5).toString();
        *poblacion = query.value(6).toString();
        *codigopostal = query.value(7).toString();
        *provincia = query.value(8).toString();
        *pais = query.value(9).toString();
        *tfno = query.value(10).toString();
        *fax = query.value(11).toString();
        *email = query.value(12).toString();
        *observaciones = query.value(13).toString();
        *ccc = query.value(14).toString();
        *cuota = query.value(15).toString();
        *venciasoc = query.value(16).toBool();
        *codvenci = query.value(17).toString();
        *tesoreria = query.value(18).toString();
        *ivaasoc = query.value(19).toBool();
        *cuentaiva = query.value(20).toString();
        *tipoiva = query.value(21).toString();
        *conceptodiario = query.value(22).toString();
        *web = query.value(23).toString();
        *claveidfiscal = query.value(24).toString();
        *procvto = query.value(25).toBool();
        *paisdat = query.value(26).toString();
        *tipo_ret = query.value(27).toString();
        *tipo_operacion_ret = query.value(28).toString();
        *ret_arrendamiento = query.value(29).toBool();
        *vfijo=query.value(30).toInt();

        *cuenta_ret_asig=query.value(31).toString();
        *es_ret_asig_arrend=query.value(32).toBool();
        *tipo_ret_asig=query.value(33).toString();
        *tipo_oper_ret_asig=query.value(34).toString();

        // bool *dom_bancaria, QString *IBAN, QString *nriesgo, QString *cedente1, QString *cedente2,
        *dom_bancaria=query.value(35).toBool(),
        *IBAN=query.value(36).toString();
        *nriesgo=query.value(37).toString();
        *cedente1=query.value(38).toString();
        *cedente2=query.value(39).toString();
        *bic=query.value(40).toString();
        *sufijo=query.value(41).toString();
        *caja_iva=query.value(42).toBool();
        *ref_mandato=query.value(43).toString();
        *firma_mandato=query.value(44).toDate();
        *cta_base_iva=query.value(45).toString();
        *imagen=basedatos::instancia()->fotocuenta(cuenta);
        return 1;
    }
    return 0;
}

void guardadatosaccesorios(QString cuenta, QString razon, QString ncomercial,QString cif,
            QString nifrprlegal, QString domicilio, QString poblacion,
            QString codigopostal, QString provincia, QString pais,
            QString tfno, QString fax, QString email, QString observaciones,
            QString ccc, QString cuota, bool ivaasoc, QString tipoiva, QString cuentaiva, QString ctabaseiva,
            bool venciasoc, QString tipovenci, QString cuentatesor, QString conceptodiario,
            QString web, QString codclaveidfiscal, bool procvto,
            QString pais_dat, QString qtipo_ret, bool ret_arrendamiento,
            QString tipo_operacion_ret, int vdiafijo,
            QString cuenta_ret_asig, bool es_ret_asig_arrend,
            QString tipo_ret_asig, QString tipo_oper_ret_asig,
            bool dom_bancaria, QString iban, QString nriesgo, QString cedente1, QString cedente2,
            QString bic, QString sufijo, bool caja_iva, QString ref_mandato, QDate firma_mandato,
            QString fototexto)
{
    basedatos::instancia()->guardadatosaccesorios(cuenta, razon, ncomercial,cif,
            nifrprlegal, domicilio, poblacion,
            codigopostal, provincia, pais,
            tfno, fax, email, observaciones,
            ccc, cuota, ivaasoc, tipoiva, cuentaiva, ctabaseiva, venciasoc, tipovenci, cuentatesor,
            conceptodiario, web, codclaveidfiscal, procvto,
            pais_dat, qtipo_ret, ret_arrendamiento, tipo_operacion_ret, vdiafijo,
            cuenta_ret_asig, es_ret_asig_arrend,
            tipo_ret_asig, tipo_oper_ret_asig,
            dom_bancaria, iban, nriesgo, cedente1, cedente2, bic, sufijo, caja_iva,
            ref_mandato, firma_mandato,
            fototexto);
}


void eliminadatosaccesorios(QString qcodigo)
{
    basedatos::instancia()->deleteDatossubcuentacuenta(qcodigo);
}

bool conanalitica(void)
{
    return basedatos::instancia()->selectAnaliticaconfiguracion();
}

bool conanalitica_parc(void)
{
    return basedatos::instancia()->selectAnalitica_parc_configuracion();
}

bool codigo_en_analitica(QString codigo)
{
  QString cadcodigos=basedatos::instancia()->select_codigos_en_ctas_analitica();
  cadcodigos.remove(' ');
  QStringList lista;
  lista = cadcodigos.split(",");
  for (int veces=0;veces<lista.size();veces++)
      if (codigo.startsWith(lista.at(veces))) return true;
  return false;
}

bool conigic(void)
{
    return basedatos::instancia()->selectIgicconfiguracion();
}
int existecodigoconcepto(QString cadena,QString *qdescrip)
{
    return basedatos::instancia()->existecodigoconcepto(cadena,qdescrip);
}

void guardacambiosconcepto(QString codigo,QString descripcion)
{
    basedatos::instancia()->updateConceptosdescripcionclave(codigo,descripcion);
}

void insertaenconceptos(QString codigo,QString descripcion)
{
    basedatos::instancia()->insertConceptos(codigo,descripcion);
}

void eliminaconcepto(QString qcodigo)
{
    basedatos::instancia()->deleteConceptosclave(qcodigo);
}


int existeclavetipoiva(QString cadena,double *tipo,double *re,QString *qdescrip) {
    QSqlQuery query = basedatos::instancia()->selectTodotiposivaclave(cadena);
 
    if ( (query.isActive()) && (query.first()) )
    {
        *qdescrip=query.value(3).toString();
	    *tipo=query.value(1).toDouble();
	    *re=query.value(2).toDouble();
        return 1;
    }
    return 0;
}


QString clave_iva_defecto(void)
{
    return basedatos::instancia()->selectClave_iva_defectoconfiguracion();
}

QString formateanumero(double numero,bool comadecimal, bool decimales)
{

 int ndecimales=2;
 if (!decimales) ndecimales=0;
 QString cadena; 
 cadena.setNum(numero,'f',ndecimales);
 if (comadecimal)
     return convacoma(cadena);
 return cadena;

}

int escuentadegasto(const QString subcuenta)
{
    return basedatos::instancia()->escuentadegasto(subcuenta);
}


int escuentadeingreso(const QString subcuenta)
{
    return basedatos::instancia()->escuentadeingreso(subcuenta);
}

int existecodigosaldosubcuenta(QString cadena)
{
    return basedatos::instancia()->existecodigosaldosubcuenta(cadena);
}

void insertasaldosubcuenta(QString codigo)
{
    basedatos::instancia()->insertSaldossubcuenta(codigo,"0","0","0");
}


QString cuentadeivasoportado()
{
    return basedatos::instancia()->cuentadeivasoportado();
}


QString cuentadeivarepercutido()
{
    return basedatos::instancia()->cuentadeivarepercutido();
}


void borraasientosnotransac(QString asientoinicial,QString asientofinal, QString ejercicio)
{
 basedatos::instancia()->borraasientosnotransac(asientoinicial,asientofinal,ejercicio);
}


bool buscaci(QString codigo, QString *descripcion, QString *qnivel)
{
    return basedatos::instancia()->selectTodoCicodigo(codigo,descripcion,qnivel);
}

void guardacambiosci(QString codigo,QString descripcion,int nivel)
{
    basedatos::instancia()->updateCidescripcioncodigo(codigo,descripcion,nivel);
}

void insertaenci(QString codigo,QString descripcion,int nivel)
{
    basedatos::instancia()->insertCi(codigo,descripcion,nivel);
}

void eliminaci(QString codigo)
{
    basedatos::instancia()->deleteCicodigo(codigo);
}

bool existeciendiario(QString codigo, int nivel)
{
    return basedatos::instancia()->existeciendiario(codigo,nivel);
}



bool existecienplanamort(QString codigo, int nivel)
{
    return basedatos::instancia()->existecienplanamort(codigo,nivel);
}


bool ciok(QString ci)
{
  if (ci.length()<3) return false;
  if (ci.length()>3 && ci.length()<6) return false;
  // Extraemos primer trozo
  QString descripcion,qnivel;
  int nnivel;
  if (buscaci(ci.left(3), &descripcion, &qnivel))
      {
       nnivel=qnivel.toInt();
       if (nnivel!=1) return false;
      }
      else return false;

  if (ci.length()==3) return true;

  if (buscaci(ci.mid(3,3), &descripcion, &qnivel))
      {
       nnivel=qnivel.toInt();
       if (nnivel!=2) return false;
      }
      else return false;

  if (ci.length()==6) return true;

  if (buscaci(ci.right(ci.length()-6),&descripcion,&qnivel))
     {
       nnivel=qnivel.toInt();
       if (nnivel!=3) return false;
     }
     else return false;

 return true;
}

QString subcuentaanterior(QString qcodigo)
{
    return basedatos::instancia()->subcuentaanterior(qcodigo);
}


QString subcuentaposterior(QString qcodigo)
{
    return basedatos::instancia()->subcuentaposterior(qcodigo);
}

bool asientoenaib(QString asiento, QString ejercicio)
{
    return basedatos::instancia()->asientoenaib(asiento,ejercicio);}

bool asientoeneib(QString asiento, QString ejercicio)
{
    return basedatos::instancia()->asientoeneib(asiento, ejercicio);
}

int ejerciciocerrando(QString ejercicio)
{
    return basedatos::instancia()->selectCerrandoejercicios(ejercicio);
}

QDate fechadeasiento(QString qasiento,QString ejercicio)
{
    return basedatos::instancia()->selectFechadiarioasiento(qasiento, ejercicio);
}


QString asientoabsdepase(QString pase)
{
    return basedatos::instancia()->selectAsientodiariopase(pase);
}


QString diariodeasiento(QString qasiento, QString ejercicio)
{
    return basedatos::instancia()->selectDiariodiarioasiento(qasiento, ejercicio);
}


int existeasiento(QString asiento, QString ejercicio)
{
    return basedatos::instancia()->existeasiento(asiento,ejercicio);
}

QString diario_apertura()
{
 return QObject::tr("APERTURA");
}

QString diario_general()
{
 return QObject::tr("GENERAL");
}

QString diario_no_asignado()
{
 return QObject::tr("SIN_ASIGNAR");
}

QString diario_cierre()
{
 return QObject::tr("CIERRE");
}

QString previsiones()
{
 return QObject::tr("PREV");
}

QString diario_regularizacion()
{
 return QObject::tr("REGULARIZACION");
}

QString msjdiario_regularizacion()
{
 return QObject::tr("REGULARIZACIÓN");
}

QString dirtrabajodefecto()
{
   QString directorio;
   if (WINDOWS) directorio=directorio.fromLocal8Bit(getenv("USERPROFILE"));
     else
       directorio=getenv("HOME");
   // if (es_os_x()) return directorio+"/Library/Application Support/keme5";
   QString nombredir="/keme";
   nombredir[0]=QDir::separator();
   directorio+=nombredir;
   return directorio;
}

void check_dir_trabajo()
{
    QString nombrefichcompleto=trayconfig();
    QString nombre="/keme.cfg";
    nombre[0]=QDir::separator();
    nombrefichcompleto+=nombre;
    if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
    QFile ficheroini(nombrefichcompleto);
    QString qdirtrabajo;

     if ( ficheroini.open( QIODevice::ReadOnly ) ) {
          QTextStream stream( &ficheroini );
          stream.setCodec("UTF-8");
          QString linea,variable,contenido;
          while ( !stream.atEnd() ) {
              linea = stream.readLine(); // linea de texto excluyendo '\n'
              // analizamos la linea de texto
              if (linea.contains('=')) {
                 variable=linea.section('=',0,0);
                 contenido=linea.section('=',1,1);
                 if (variable.compare("DIRECTORIO")==0) qdirtrabajo=contenido;
                }
              }
          ficheroini.close();
         }
    if (qdirtrabajo.length()==0) qdirtrabajo=dirtrabajodefecto();
    QDir a(qdirtrabajo);
    qdirtrabajo.append(QDir::separator());
    if (!a.exists())
       {
        a.mkdir(qdirtrabajo);
        // comprobamos que exista la empresa de ejemplo SQLITE
        QString fichsqlite=qdirtrabajo+"ejemplo-sqlite";
        if (!QFile::exists(fichsqlite))
           {
              QString pasa2;
              pasa2=":/fich/fich/";
              //pasa2=traydoc();
              //pasa2.append(QDir::separator());
              pasa2.append("ejemplo-sqlite");
              if (eswindows()) pasa2=QFile::encodeName(pasa2);
              QFile fichero(pasa2);
              if (eswindows()) fichero.copy(QFile::encodeName(fichsqlite));
                  else fichero.copy(fichsqlite);
           }
        // comprobamos que exista el fichero de la memoria
        QString fichmemoria=qdirtrabajo+"memoria.tex";
        if (!QFile::exists(fichmemoria))
           {
              QString pasa2;
              pasa2=":/fich/fich/";
              //pasa2=traydoc();
              //pasa2.append(QDir::separator());
              pasa2.append("memoria.tex");
              if (eswindows()) pasa2=QFile::encodeName(pasa2);
              QFile fichero(pasa2);
              if (eswindows()) fichero.copy(QFile::encodeName(fichmemoria));
                  else fichero.copy(fichmemoria);
           }
       }

}

QString dirtrabajo(void)
 {
  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;
  if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
  QFile ficheroini(nombrefichcompleto);
  QString qdirtrabajo;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("DIRECTORIO")==0) qdirtrabajo=contenido;
              }
            }
        ficheroini.close();
       }
  if (qdirtrabajo.length()==0) qdirtrabajo=dirtrabajodefecto();
  QDir a(qdirtrabajo);
  qdirtrabajo.append(QDir::separator());
  if (!a.exists())
     {
      a.mkdir(qdirtrabajo);
      // comprobamos que exista la empresa de ejemplo SQLITE
      QString fichsqlite=qdirtrabajo+"ejemplo-sqlite";
      if (!QFile::exists(fichsqlite))
         {
            QString pasa2;
            pasa2=":/fich/fich/";
            //pasa2=traydoc();
            //pasa2.append(QDir::separator());
            pasa2.append("ejemplo-sqlite");
            if (eswindows()) pasa2=QFile::encodeName(pasa2);
            QFile fichero(pasa2);
            if (eswindows()) fichero.copy(QFile::encodeName(fichsqlite));
                else fichero.copy(fichsqlite);
         }
      // comprobamos que exista el fichero de la memoria
      QString fichmemoria=qdirtrabajo+"memoria.tex";
      if (!QFile::exists(fichmemoria))
         {
            QString pasa2;
            pasa2=":/fich/fich/";
            //pasa2=traydoc();
            //pasa2.append(QDir::separator());
            pasa2.append("memoria.tex");
            if (eswindows()) pasa2=QFile::encodeName(pasa2);
            QFile fichero(pasa2);
            if (eswindows()) fichero.copy(QFile::encodeName(fichmemoria));
                else fichero.copy(fichmemoria);
         }
     }
  // -----------------------------------------------------
  if (nombrebd().length()>0)
     {
      qdirtrabajo.append(nombrebd());
      if (es_sqlite()) qdirtrabajo=nombrebd(); // el nombre de la base de datos guarda toda la ruta
      qdirtrabajo.append("-dat");
     }
  if (eswindows()) qdirtrabajo=QFile::encodeName(qdirtrabajo);
  QDir d(qdirtrabajo);
  if (!d.exists()) d.mkdir(qdirtrabajo);
  return qdirtrabajo;
}

QString dirtrabajodocs(QString qdoc)
{
    QString dir=dirtrabajo();
    dir.append(QDir::separator());
    dir.append(qdoc);
    QDir d(dir);
    if (!d.exists()) d.mkdir(dir);
    return dir;
}

QString dirtrabajobd(void)
{
  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;
  if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
  QFile ficheroini(nombrefichcompleto);
  QString qdirtrabajo;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("DIRECTORIO")==0) qdirtrabajo=contenido;
              }
            }
        ficheroini.close();
       }
  if (qdirtrabajo.length()==0) qdirtrabajo=dirtrabajodefecto();
  if (eswindows()) qdirtrabajo=QFile::encodeName(qdirtrabajo);
  QDir d(qdirtrabajo);
  if (!d.exists()) d.mkdir(qdirtrabajo);
  return qdirtrabajo;
  // return dirtrabajo();
}


int idiomaconfig(void)
 {
  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;
  if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
  QFile ficheroini(nombrefichcompleto);
  int idioma=0;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("IDIOMA")==0) idioma=contenido.toInt();
              }
            }
        ficheroini.close();
       }
  return idioma;
}

QString fich_catalan()
{
 QString nombre=":/translate/";
 nombre.append(QDir::separator());
 nombre+="keme4_cat";
 if (eswindows()) nombre=QFile::encodeName(nombre);
 return nombre;
}

QString fich_gallego()
{
 QString nombre=":/translate/";
 nombre.append(QDir::separator());
 nombre+="keme4_gl";
 if (eswindows()) nombre=QFile::encodeName(nombre);
 return nombre;
}


QString fich_ingles()
{
 QString nombre=":/translate/";
 nombre.append(QDir::separator());
 nombre+="keme4_eng";
 if (eswindows()) nombre=QFile::encodeName(nombre);
 return nombre;
}


QString fich_portugues()
{
 QString nombre=":/translate/";
 nombre.append(QDir::separator());
 nombre+="keme4_pt";
 if (eswindows()) nombre=QFile::encodeName(nombre);
 return nombre;
}

QString fich_euskera()
{
 QString nombre=":/translate/";
 nombre.append(QDir::separator());
 nombre+="keme4_eus";
 if (eswindows()) nombre=QFile::encodeName(nombre);
 return nombre;
}


QString filtracad(QString elorigen)
{
  QString origen=elorigen;
  origen.replace(34,39); // covertimos a comillas simples

  QString caracteres="ABCDEFGHIJKLMNÑOPQRSTUVWXYZabcdefghijklmnñopqrstuvxyz 1234567890áéíóú"
                     "àèìòùäëïöüÁÉÍÓÚÀÈÌÒÙÄËÏÖÜçÇâêîôûÂÊÎÔÛ!$%&/()='?¡¿*-+_.:,;{}[]^<>ºª€~\\";
  QString destino;
  destino=origen;
  for (int veces=0; veces<origen.length(); veces++)
     {
       if (!caracteres.contains(origen[veces]))
           destino=destino.remove(origen[veces]);
     }
  destino.replace("\\","/");
  //destino.remove("º");
  //destino.remove("ª");
  destino.replace("#","\\#");
  destino.replace("$","\\$");
  destino.replace("%","\\%");
  destino.replace("^","\\^");
  destino.replace("&","\\&");
  destino.replace("{","\\{");
  destino.replace("}","\\}");
  destino.replace("~","\\~");
  destino.replace("_","\\_");
  destino.replace("<","\\flq");
  destino.replace(">","\\frq");
  destino.replace("€","\\texteuro ");
  return destino;
}


QString mcortaa(QString cadena, uint num)
{
    uint longitud=0;
    QString mays=cadena.toUpper();
    QString numeros="0123456789.,' ";
    float medias=0;
    for (int veces=0; veces<int(cadena.length()); veces++)
        {
	    longitud++;
	    if (cadena[veces]==mays[veces]
                && numeros.contains(cadena[veces])==0) 
	       {
		   medias=medias+0.5;
	       }
	    if (longitud+uint(medias)>=num)
	        {
    	         QString devuelve=cadena; devuelve.truncate(uint(veces));
                 return devuelve;
	        } 
	}
    return cadena;
}


QString programa_imprimir()
{

  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;
  
  if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
  QFile ficheroini(nombrefichcompleto);
  QString qimprimir;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("IMPRIMIR")==0) qimprimir=contenido;
              }
            }
        ficheroini.close();
       }
  if (qimprimir.length()==0) qimprimir=programa_imprimir_defecto();
  return qimprimir;

}


QString programa_imprimir_defecto()
{
  return "okular";
}


QString visordvidefecto()
{
  if (WINDOWS) return "yap";
  return "okular";
}

QString visordvi()
{
  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;

  if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
  QFile ficheroini(nombrefichcompleto);
  QString qvisor;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("VISORDVI")==0) qvisor=contenido;
              }
            }
        ficheroini.close();
       }
  if (qvisor.length()==0) qvisor=visordvidefecto();
  return qvisor;

}


bool filtrartexaxtivado()
{
  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;

  if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
  QFile ficheroini(nombrefichcompleto);
  QString cnum;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("FILTRARTEX")==0) cnum=contenido;
              }
            }
        ficheroini.close();
       }
       // else { if ( WINDOWS ) return true; }
  if (cnum.length()==0) return false;

  return cnum.toInt();

}

QString adapta(QString cadena)
{
 if (eswindows()) return QFile::encodeName(cadena);
   else return cadena;
}

void filtratex(QString cadfich)
{

 QString qfichero=cadfich;
 QString fichmod=qfichero+"2";

 QFile escritura( adapta(fichmod) );

  if ( !escritura.open( QIODevice::WriteOnly ) ) return; 
  QTextStream streamescritura( &escritura );
  streamescritura.setCodec("UTF-8");


 QFile lectura( adapta(qfichero)  );
 
   if ( lectura.open(  QIODevice::ReadOnly  ) )
      {
        QTextStream streamlectura( &lectura );
        streamlectura.setCodec("UTF-8");
        QString linea;
         while ( !streamlectura.atEnd() )
	   {
	     linea = streamlectura.readLine(); // linea de texto, excluye '\n'
             if (linea.length()==0) continue;
             if (linea.contains("usepackage{ucs}")) continue;
             if (linea.contains("usepackage[utf8x]")) continue;
             linea.replace(QObject::tr("á"),QString("\\'a"));
             linea.replace(QObject::tr("é"),QString("\\'e"));
             linea.replace(QObject::tr("í"),QString("\\'i"));
             linea.replace(QObject::tr("ó"),QString("\\'o"));
             linea.replace(QObject::tr("ú"),QString("\\'u"));
             linea.replace(QObject::tr("Á"),QString("\\'A"));
             linea.replace(QObject::tr("É"),QString("\\'E"));
             linea.replace(QObject::tr("Í"),QString("\\'I"));
             linea.replace(QObject::tr("Ó"),QString("\\'O"));
             linea.replace(QObject::tr("Ú"),QString("\\'U"));
             linea.replace(QObject::tr("ñ"),QString("\\~n"));
             linea.replace(QObject::tr("Ñ"),QString("\\~N"));
             linea.replace(QObject::tr("à"),QString("\\`a"));
             linea.replace(QObject::tr("è"),QString("\\`e"));
             linea.replace(QObject::tr("ì"),QString("\\`i"));
             linea.replace(QObject::tr("ò"),QString("\\`o"));
             linea.replace(QObject::tr("ù"),QString("\\`u"));
             linea.replace(QObject::tr("À"),QString("\\`A"));
             linea.replace(QObject::tr("È"),QString("\\`E"));
             linea.replace(QObject::tr("Ì"),QString("\\`I"));
             linea.replace(QObject::tr("Ò"),QString("\\`O"));
             linea.replace(QObject::tr("Ù"),QString("\\`U"));
             linea.replace(QObject::tr("ã"),QString("\\~a"));
             linea.replace(QObject::tr("ẽ"),QString("\\~e"));
             linea.replace(QObject::tr("ĩ"),QString("\\~i"));
             linea.replace(QObject::tr("õ"),QString("\\~o"));
             linea.replace(QObject::tr("ũ"),QString("\\~u"));
             linea.replace(QObject::tr("Ã"),QString("\\~A"));
             linea.replace(QObject::tr("Ẽ"),QString("\\~E"));
             linea.replace(QObject::tr("Ĩ"),QString("\\~I"));
             linea.replace(QObject::tr("Õ"),QString("\\~O"));
             linea.replace(QObject::tr("Ũ"),QString("\\~U"));
             linea.replace(QObject::tr("â"),QString("\\^a"));
             linea.replace(QObject::tr("ê"),QString("\\^e"));
             linea.replace(QObject::tr("î"),QString("\\^i"));
             linea.replace(QObject::tr("ô"),QString("\\^o"));
             linea.replace(QObject::tr("û"),QString("\\^u"));
             linea.replace(QObject::tr("Â"),QString("\\^A"));
             linea.replace(QObject::tr("Ê"),QString("\\^E"));
             linea.replace(QObject::tr("Î"),QString("\\^I"));
             linea.replace(QObject::tr("Ô"),QString("\\^O"));
             linea.replace(QObject::tr("Û"),QString("\\^U"));
             linea.replace(QObject::tr("ç"),QString("\\c c"));
             linea.replace(QObject::tr("Ç"),QString("\\c C"));
             if (!linea.startsWith("%")) linea.replace(QObject::tr("%"),QString("\\%"));
             streamescritura << linea << "\n";
           }
        lectura.close();
       } else return;
  escritura.close();

 //queda copiar escritura en lectura y borrar escritura
 QDir directorio(adapta(dirtrabajo()));
 directorio.remove(adapta(qfichero));
 directorio.rename(adapta(fichmod),adapta(qfichero));

}



bool haycomadecimal()
{
  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;

  QFile ficheroini(adapta(nombrefichcompleto));
  QString cnum;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("ESTILONUMERICO")==0) cnum=contenido;
              }
            }
        ficheroini.close();
       }
  if (cnum.length()==0) return true;

  return cnum.toInt();

}


bool haydecimales()
{
  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;

  QFile ficheroini(adapta(nombrefichcompleto));
  QString cnum;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("SINDECIMALES")==0) cnum=contenido;
              }
            }
        ficheroini.close();
       }
  if (cnum.length()==0) return true;

  return !cnum.toInt();

}


QString splash_pref()
{
  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;

  QFile ficheroini(adapta(nombrefichcompleto));
  QString cadena;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("SPLASH")==0) cadena=contenido;
              }
            }
        ficheroini.close();
       }

  return cadena;

}


QString formatea_redondeado_sep(double numero,bool comadecimal, bool decimales)
{
    numero=CutAndRoundNumberToNDecimals (numero, 2);
    return formateanumerosep(numero,comadecimal,decimales);
}

QString formatea_redondeado(double numero,bool comadecimal, bool decimales)
{
    numero=CutAndRoundNumberToNDecimals (numero, 2);
    return formateanumero(numero,comadecimal,decimales);
}

QString formateanumerosep(double numero,bool comadecimal, bool decimales)
{
 QString cad; 
 cad.setNum(numero,'f',2);
 if (comadecimal) cad=convacoma(cad);

 if (comadecimal)
     {
       if (decimales)
           return puntuanumerocomadec(cad);
        else
             {
               QString cad2=puntuanumerocomadec(cad);
               cad2.truncate(cad2.length()-3);
               return cad2;
             }
     }
     else
        {
          if (decimales)
              return puntuanumeropuntodec(cad);
           else
                {
                  QString cad2=puntuanumeropuntodec(cad);
                  cad2.truncate(cad2.length()-3);
                  return cad2;
                }
        }

}


bool cifiltrook(QString ci)
{
  if (ci.length()<3) return false;
  if (ci.length()>3 && ci.length()<6) return false;
  // Extraemos primer trozo
  QString descripcion,qnivel;
  int nnivel;
  if (ci.left(3)!=QString("???"))
   {
    if (buscaci(ci.left(3), &descripcion, &qnivel))
       {
        nnivel=qnivel.toInt();
        if (nnivel!=1) return false;
       }
      else return false;
    }

  if (ci.length()==3) return true;

  if (ci.mid(3,3)!=QString("???"))
    {
     if (buscaci(ci.mid(3,3), &descripcion, &qnivel))
       {
        nnivel=qnivel.toInt();
        if (nnivel!=2) return false;
       }
      else return false;
     }

  if (ci.length()==6) return true;

  if (ci[6]!='*' && ci.length()!=7)
   {
    if (buscaci(ci.right(ci.length()-6),&descripcion,&qnivel))
     {
       nnivel=qnivel.toInt();
       if (nnivel!=3) return false;
     }
     else return false;
   }

 return true;
}


QString filtroci(QString codigo, bool diario_ci)
{
    QString filtro;
    QString adon;
    if (diario_ci) adon="diario_ci.";

    if (!codigo.startsWith("???") && codigo.length()>2)
    {
      if (codigo.length()==3)
        filtro=adon+"ci='"+codigo.replace("'","''")+"'";
      else
       {
        if (basedatos::instancia()->cualControlador() == basedatos::SQLITE) {
            filtro = adon+"ci like '"+ codigo.left(3).replace("'","''") +"%'";
        }
        else {
            filtro = "position('"+ codigo.left(3).replace("'","''") +"' in "+adon+"ci)=1";
        }
       }
    }

    if (codigo.startsWith("???") && codigo.length()==3)
    {
        if (basedatos::instancia()->cualControlador() == basedatos::SQLITE) {
            filtro = "length("+adon+"ci)=3";
        }
        else {
            filtro = "char_length("+adon+"ci)=3";
        }
    }


    if (codigo.length()>5 && codigo.mid(3,3)!=QString("???"))
    {
        if (basedatos::instancia()->cualControlador() == basedatos::SQLITE) {
            if (filtro.length()>0) filtro += " and ";
            else filtro = "";
            filtro += adon+"ci like '___"+ codigo.mid(3,3).replace("'","''") +"%'";
        }
        else {
            if (filtro.length()>0) filtro += " and position('";
            else filtro = "position('";
            filtro += codigo.mid(3,3).replace("'","''") +"' in "+adon+"ci)=4";
        }
    }

    if (codigo.length()==6)
    {
        if (basedatos::instancia()->cualControlador() == basedatos::SQLITE) {
            if (filtro.length()>0) filtro += " and ";
            filtro += "length("+adon+"ci)=6";
        }
        else {
            if (filtro.length()>0) filtro += " and ";
            filtro += "char_length("+adon+"ci)=6";
        }
    }

    if (codigo.length()>6 && codigo[6]!='*')
    {
        if (basedatos::instancia()->cualControlador() == basedatos::SQLITE) {
            if (filtro.length()>0) filtro += " and ";
            filtro+= "substr("+adon+"ci,7,length("+adon+"ci)-6)='";
            filtro+= codigo.right(codigo.length()-6).replace("'","''");
            filtro+="'";
            // filtro += adon+"ci like '______"+ codigo.right(codigo.length()-6).replace("'","''") +"%'";
        }
        else {
            if (filtro.length()>0) filtro += " and ";
            filtro+="char_length("+adon+"ci)>5 and ";
            filtro+="substring("+adon+"ci from 7 for char_length("+adon+"ci)-6)='";
            filtro+=codigo.right(codigo.length()-6).replace("'","''");
            filtro+="'";
            /*
            filtro += "position('";
            filtro += codigo.right(codigo.length()-6).replace("'","''");
            filtro += "' in "+adon+"ci)=7";*/
        }
     }

    if (basedatos::instancia()->cualControlador() == basedatos::SQLITE) {
        if (filtro.length()==0) filtro = "length("+adon+"ci)>0";
    }
    else {
        if (filtro.length()==0) filtro = "char_length("+adon+"ci)>0";
    }

    return filtro;
}


bool parentesisbalanceados(QString cadena)
{
  evaluadorparentesis evparentesis;
  evparentesis.inicializa(cadena);
  if (!evparentesis.okparentesis()) return 0;
  return 1;
}

QString cadenamedia()
{
 return QObject::tr("MEDIA");
}

QString cadpositivo()
{
  return QObject::tr(";POSITIVO");
}


QString tipoNumerico()
{
  return QObject::tr("Numérico");
}

QString tipoFecha()
{
  return QObject::tr("Fecha");
}

QString tipoCaracteres()
{
  return QObject::tr("Caracteres");
}

QString tipoSubcuenta()
{
  return QObject::tr("Subcuenta");
}

QString tipoNIF()
{
  return QObject::tr("NIF");
}


QString tipoCI()
{
  return QObject::tr("CI");
}


QString tipoDEBE()
{
  return QObject::tr("DEBE");
}

QString tipoHABER()
{
  return QObject::tr("HABER");
}



bool chequeaformula(QString formula)
{
  if (formula.length()==0) return 1;
  if (!parentesisbalanceados(formula)) return 0;
  if (formula.contains("(+")) return 0;
  if (formula.contains("(/")) return 0;
  if (formula.contains("(*")) return 0;
  if (formula.contains("/)")) return 0;
  if (formula.contains("*)")) return 0;
  if (formula.contains("+)")) return 0;
  if (formula.contains("-)")) return 0;
  if (formula.contains("()")) return 0;
  if (formula.contains(")(")) return 0;
  if (formula.contains("++")) return 0;
  if (formula.contains("+-")) return 0;
  if (formula.contains("+*")) return 0;
  if (formula.contains("+/")) return 0;
  if (formula.contains("--")) return 0;
  if (formula.contains("-+")) return 0;
  if (formula.contains("-*")) return 0;
  if (formula.contains("-/")) return 0;
  if (formula.contains("**")) return 0;
  if (formula.contains("*+")) return 0;
  if (formula.contains("*/")) return 0;
  if (formula.contains("//")) return 0;
  if (formula.contains("/+")) return 0;
  if (formula.contains("/*")) return 0;
  if (formula.endsWith("+")) return 0;
  if (formula.endsWith("-")) return 0;
  if (formula.endsWith("/")) return 0;
  if (formula.endsWith("*")) return 0;
  if (formula.endsWith("(")) return 0;
  
  if (formula.contains("][")) return 0;
  if (formula.contains("[[")) return 0;
  if (formula.contains("]]")) return 0;
  if (formula.contains("[]")) return 0;
    
  if (formula.indexOf(']',0)<formula.indexOf('[',0)) return 0;
  
  int indice=0;
  while (indice<formula.length() && formula.indexOf('[',indice)>=0)
    {
             int base=formula.indexOf('[',indice);
             if (formula.indexOf(']',indice)<0) return 0;
             if (base+1<formula.length() && formula.indexOf('[',base+1)>=0)
                  if (formula.indexOf('[',base+1)<formula.indexOf(']',base)) return 0; 
             indice=formula.indexOf(']',indice)+1;
             if (indice<formula.length() && formula.indexOf(']',indice)>=0 && 
             	 (formula.indexOf(']',indice)<formula.indexOf('[',indice) || formula.indexOf('[',indice)<0))
                 return 0;
    }

// hacer lo propio con las llaves  
  if (formula.contains("}{")) return 0;
  if (formula.contains("{{")) return 0;
  if (formula.contains("}}")) return 0;
  if (formula.contains("{}")) return 0;

  if (formula.indexOf('}',0)<formula.indexOf('{',0)) return 0;
  
  indice=0;
  while (indice<formula.length() && formula.indexOf('{',indice)>=0)
    {
      int base=formula.indexOf('{',indice);
      if (formula.indexOf('}',indice)<0) return 0;
      if (base+1<formula.length() &&formula.indexOf('{',base+1)>=0)
	  if (formula.indexOf('{',base+1)<formula.indexOf('}',base)) return 0; 
      indice=formula.indexOf('}',indice)+1;
      if (indice<formula.length() && formula.indexOf('}',indice)>=0 && 
	  (formula.indexOf('}',indice)<formula.indexOf('{',indice) || formula.indexOf('{',indice)<0)) return 0;
    }

// -------------------------------------------------------------  
  QString cadpru;
  cadpru=formula;
  int final;
  int inicio;
  indice=0;
  while (indice<cadpru.length())
      {
         if (cadpru[indice]=='[')
           {
              inicio=indice;
              final=inicio;
              while (final<cadpru.length() && cadpru[final]!=']') final++;
              cadpru.remove(inicio,final-inicio+1);
              continue;
           }
        if (cadpru[indice]=='{')
           {
              inicio=indice;
              final=inicio;
              while (final<cadpru.length() && cadpru[final]!='}') final++;
              cadpru.remove(inicio,final-inicio+1);
              continue;
           }
       indice++;
      }
    
  QString cadcods="()+-/*0123456789.?:<=!>siSI";
  
  indice=0;
  cadpru.remove(' ');
  while (indice<cadpru.length())
      {
        if (!cadcods.contains(cadpru[indice])) return 0;
        indice++;
      }
 
  return 1;   

}

void cadvalorestipoiva(QString cadena,QString *tipo,QString *re)
{
    basedatos::instancia()->selectTiporetiposivaclave(cadena,tipo,re);
}

bool cadnumvalido(QString cadnum)
{
  QString num;
  num=cadnum.trimmed();
  QString digitos="0123456789.-";
  for (int veces=0;veces<num.length();veces++)
    {
      if (!digitos.contains(num[veces])) return false;
    }
  if (num.count('.')>1) return false;
  if (num.count('-')>1) return false;
  if (num.count('-')==1 && num.indexOf('-')>0) return false;
  return true;
}

double tipoivaefectivo(QString clave)
{
    QString qtipo, qre;
    double tipo=0,re=0;

    if ( basedatos::instancia()->selectTiporetiposivaclave(clave,&qtipo,&qre) ) {
        tipo = qtipo.toDouble();
        re = qre.toDouble();
        return ( (tipo+re)/100 );
    }
    return 0;
}

double tipoivanore(QString clave)
{
    QString qtipo, qre;
    double tipo=0;

    if ( basedatos::instancia()->selectTiporetiposivaclave(clave,&qtipo,&qre) ) {
        tipo = qtipo.toDouble();
        return ( tipo/100 );
    }
    return 0;
}


bool cadfechavalida(QString cadfecha)
{
  // formato dd/mm/aaaa

  // 1 comprobamos longitud
  if (cadfecha.length()!=10) return false;

  // comprobamos separadores
  if (cadfecha[2]!='/' && cadfecha[2]!='-' &&cadfecha[2]!='.') return false;

  if (cadfecha[2]=='/' && cadfecha[5]!='/') return false;
  if (cadfecha[2]=='-' && cadfecha[5]!='-') return false;
  if (cadfecha[2]=='.' && cadfecha[5]!='.') return false;

  // comprobaciones básicas
  // en las demás posiciones deben de haber números
  QString numeros="0123456789";
  if (!numeros.contains(cadfecha[0])) return false;
  if (!numeros.contains(cadfecha[1])) return false;
  if (!numeros.contains(cadfecha[3])) return false;
  if (!numeros.contains(cadfecha[4])) return false;
  if (!numeros.contains(cadfecha[6])) return false;
  if (!numeros.contains(cadfecha[7])) return false;
  if (!numeros.contains(cadfecha[8])) return false;
  if (!numeros.contains(cadfecha[9])) return false;

  // días, meses y años han de ser enteros positivos
  int dia,mes,anyo;
  QString cadextract;

  cadextract=cadfecha.mid(0,2);
  dia=cadextract.toInt();

  cadextract=cadfecha.mid(3,2);
  mes=cadextract.toInt();

  cadextract=cadfecha.mid(6,4);
  anyo=cadextract.toInt();
  
  if (QDate::isValid(anyo,mes,dia)) return true;
    else return false;

}


bool cuentaycodigoenplanamort(QString cuenta,QString codigo)
{
    return basedatos::instancia()->cuentaycodigoenplanamort(cuenta,codigo);
}


bool practamort(QString ejercicio)
{
    return basedatos::instancia()->practamort(ejercicio);
}

void actualizasaldo(QString cuenta,QString importe,bool cargo)
{
    basedatos::instancia()->updateSaldossubcuentasaldocargocodigo(cuenta,importe,cargo);
}


double saldocuentaendiario(QString cuenta)
{
    return basedatos::instancia()->selectDebehaberdiariocuenta(cuenta);
}


double totalamortfiscal(QString cuenta)
{
    return basedatos::instancia()->selectSumDotacion_fiscalamortfiscalycontablecuenta_activo(cuenta);
}

QString ciamort()
{
 return QObject::tr("AMR");
}


bool espasevtoprocesado(QString qpase)
{
    return basedatos::instancia()->espasevtoprocesado(qpase);
}


QString ejercicio_igual_o_maxigual(QDate fecha)
{
    return basedatos::instancia()->ejercicio_igual_o_maxigual(fecha);
}

QString numasientovto(QString vto)
{   
    return basedatos::instancia()->numasientovto(vto);
}

QString numasientovtovto(QString vto)
{
    return basedatos::instancia()->numasientovtovto(vto);
}


bool vencimientoprocesado(QString vto)
{
    return basedatos::instancia()->selectPendientevencimientosnum(vto);
}


bool existevencimiento(QString vto)
{
    return basedatos::instancia()->existevencimiento(vto);
}

void anulavencimiento(QString vto)
{
    basedatos::instancia()->updateVencimientosanuladonum(vto,true);
}


void borravenci(QString vto)
{
    basedatos::instancia()->deleteVencimientosnum(vto);
}


QString cadmes(int mes)
{
  switch (mes)
    {
      case 1 : return QObject::tr("ENERO");
         break;
      case 2 : return QObject::tr("FEBRERO");
         break;
      case 3 : return QObject::tr("MARZO");
         break;
      case 4 : return QObject::tr("ABRIL");
         break;
      case 5 : return QObject::tr("MAYO");
         break;
      case 6 : return QObject::tr("JUNIO");
         break;
      case 7 : return QObject::tr("JULIO");
         break;
      case 8 : return QObject::tr("AGOSTO");
         break;
      case 9 : return QObject::tr("SEPTIEMBRE");
         break;
      case 10 : return QObject::tr("OCTUBRE");
         break;
      case 11 : return QObject::tr("NOVIEMBRE");
         break;
      case 12 : return QObject::tr("DICIEMBRE");
         break;
    }
 return "";
}

QString cadmes_minus(int mes)
{
  switch (mes)
    {
      case 1 : return QObject::tr("enero");
         break;
      case 2 : return QObject::tr("febrero");
         break;
      case 3 : return QObject::tr("marzo");
         break;
      case 4 : return QObject::tr("abril");
         break;
      case 5 : return QObject::tr("mayo");
         break;
      case 6 : return QObject::tr("junio");
         break;
      case 7 : return QObject::tr("julio");
         break;
      case 8 : return QObject::tr("agosto");
         break;
      case 9 : return QObject::tr("septiembre");
         break;
      case 10 : return QObject::tr("octubre");
         break;
      case 11 : return QObject::tr("noviembre");
         break;
      case 12 : return QObject::tr("diciembre");
         break;
    }
 return "";
}

QString fechaliteral(QDate fecha)
{
 QString cadfecha,cadnum;
 cadnum.setNum(fecha.day());
 cadfecha=cadnum+" de ";
 cadfecha+=cadmes_minus(fecha.month());
 cadfecha+=" de ";
 cadnum.setNum(fecha.year());
 cadfecha+=cadnum;
 return cadfecha;
}

void borraasientomodelo(QString asiento)
{
    basedatos::instancia()->deleteDet_as_modeloasientomodelo(asiento);
    basedatos::instancia()->deleteVar_as_modeloasientomodelo(asiento);
    basedatos::instancia()->deleteCab_as_modeloasientomodelo(asiento);
}

void fijapermisos(QString ruta, QString permisos)
{
    if (WINDOWS) return;

    QStringList arguments;
    arguments << permisos;
    arguments << ruta;
    QProcess *myProcess = new QProcess(NULL);
    myProcess->start("chmod",arguments);
    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return ;
       }
    if (!myProcess->waitForFinished (1000))
       {
         delete myProcess;
         return ;
       }
}

QString cad_tiempo()
{
    QString nombrefichero;
    QDate fecha;
    nombrefichero+="-";
    nombrefichero+=fecha.currentDate().toString("yy.MM.dd");
    nombrefichero+="-";
    QTime hora;
    hora=hora.currentTime();
    nombrefichero+=hora.toString("hhmmss");
    return nombrefichero;
}

int imprimelatex(QString qfichero)
{
    if (WINDOWS) return consultalatex(qfichero);

    QProgressDialog progreso(QObject::tr("Generando informe ... ")
                             , 0, 0, 0);
    // progreso.setWindowModality(Qt::WindowModal);
    progreso.setMinimumDuration ( 0 );
    progreso.setWindowTitle(QObject::tr("Generando  ... "));;
    progreso.show();
    progreso.update();
    QApplication::processEvents();
    QApplication::processEvents();

    QString pasafich=dirtrabajo();
    pasafich.append(QDir::separator());
    pasafich.append(qfichero);
    pasafich.append(".tex");
    if (filtrartexaxtivado()) filtratex(pasafich);

    QString rutapdf=rutacopiapdf();

    QString fichero=qfichero;

    QString cadexec="pdflatex";
    QString cadarg=dirtrabajo();
    cadarg.append(QDir::separator());
    cadarg+=fichero;

    QObject *parent=NULL;

     QStringList arguments;
     arguments << adapta(cadarg);

     QProcess *myProcess = new QProcess(parent);

     myProcess-> setWorkingDirectory(adapta(dirtrabajo()));

     QApplication::processEvents();

     myProcess->start(cadexec,arguments);

     QApplication::processEvents();

    // system(cadexec);
    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return 1;
       }
    QApplication::processEvents();
    if (!myProcess->waitForFinished (60000))
       {
         delete myProcess;
         return 1;
       }

    arguments.clear();

    QApplication::processEvents();

    // ---------------------------------------------------------------
    // borramos aux y log
    QString fich_aux=cadarg+".aux";
    QString fich_log=cadarg+".log";

    QFile faux(eswindows() ? QFile::encodeName(fich_aux): fich_aux);
    faux.remove();
    QFile flog(eswindows() ? QFile::encodeName(fich_log) : fich_log);
    flog.remove();
    // ---------------------------------------------------------------


 /*
    cadexec="dvips";
    arguments << "-Pcmz";
    cadarg=dirtrabajo();
    cadarg.append(QDir::separator());
    cadarg+=fichero;
    cadarg+".dvi";
    arguments << adapta(cadarg);
    arguments << "-o";
    QString cadfich=qfichero; cadfich.append(".ps");
    arguments << cadfich;
    myProcess->start(cadexec,arguments);

    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return 2;
       }
    if (!myProcess->waitForFinished ())
       {
         delete myProcess;
         return 2;
       }

*/
    arguments.clear();
    cadexec=programa_imprimir();
    QString qfich;
    qfich=dirtrabajo();
    qfich.append(QDir::separator());
    qfich+=fichero;
    qfich+=".pdf";
 /*   if (programa_imprimir()=="kprinter")
        {
         arguments << "--caption";
         arguments << "KEME-Contabilidad";
         cadarg=dirtrabajo();
         cadarg.append(QDir::separator());
         cadarg+=fichero;
         cadarg+=".ps";
         arguments << adapta(cadarg);
         // arguments << "&";
         myProcess->start(cadexec,arguments);
         if (!myProcess->waitForStarted ())
           {
            delete myProcess;
            return 3;
           }
         myProcess->waitForFinished(-1);
         delete myProcess;
         return 0;
        }
     else */

    QApplication::processEvents();

     if (rutapdf.isEmpty() || pref_forzar_ver_pdf())
        {
          cadexec+=" ";
          cadexec+=qfich;
          if (! myProcess->startDetached(cadexec))
            {
             delete myProcess;
             return 3;
            }
        }

  delete myProcess;

  QApplication::processEvents();

  // copiar aquí pdf
  QString rpdf=rutapdf;
  if (!rpdf.isEmpty())
     {
      // qWarning(rpdf.toAscii().constData());
      // qWarning(qfich.toAscii().constData());
      QString destino=rpdf;
      destino.append(QDir::separator());
      destino.append(fichero);
      destino.append(cad_tiempo());
      destino.append(".pdf");
      QFile fich(qfich);
      if (eswindows()) fich.copy(QFile::encodeName(destino));
         else fich.copy(destino);

     }
  return 0;

}



int imprimelatex2(QString qfichero)
{
    if (WINDOWS) return consultalatex2(qfichero);

    QProgressDialog progreso(QObject::tr("Generando informe... ")
                             , 0, 0, 0);
    // progreso.setWindowModality(Qt::WindowModal);
    progreso.setMinimumDuration ( 0 );
    progreso.setWindowTitle(QObject::tr("Generando  ... "));;
    progreso.show();
    progreso.update();
    QApplication::processEvents();

    QString pasafich=dirtrabajo();
    pasafich.append(QDir::separator());
    pasafich.append(qfichero);
    pasafich.append(".tex");
    if (filtrartexaxtivado()) filtratex(pasafich);

    QString fichero=qfichero;

    QString cadexec="pdflatex";
    QString cadarg=dirtrabajo();
    cadarg.append(QDir::separator());
    cadarg+=fichero;

    QObject *parent=NULL;

     QStringList arguments;
     arguments << cadarg;

     QProcess *myProcess = new QProcess(parent);

     myProcess-> setWorkingDirectory(dirtrabajo());

     QApplication::processEvents();

     myProcess->start(cadexec,arguments);

    // system(cadexec);
    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return 1;
       }
    QApplication::processEvents();
    if (!myProcess->waitForFinished (60000))
       {
         delete myProcess;
         return 1;
       }

     //segunda pasada
     myProcess->start(cadexec,arguments);
     QApplication::processEvents();

    // system(cadexec);
    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return 1;
       }
    QApplication::processEvents();
    if (!myProcess->waitForFinished (60000))
       {
         delete myProcess;
         return 1;
       }

    QApplication::processEvents();

    // ---------------------------------------------------------------
    // borramos aux y log
    QString fich_aux=cadarg+".aux";
    QString fich_log=cadarg+".log";

    QFile faux(eswindows() ? QFile::encodeName(fich_aux): fich_aux);
    faux.remove();
    QFile flog(eswindows() ? QFile::encodeName(fich_log) : fich_log);
    flog.remove();
    // ---------------------------------------------------------------


/*    arguments.clear();
    cadexec="dvips";
    arguments << "-Pcmz";
    cadarg=dirtrabajo();
    cadarg.append(QDir::separator());
    cadarg+=fichero;
    cadarg+".dvi";
    arguments << cadarg;
    arguments << "-o";
    QString cadfich=qfichero; cadfich.append(".ps");
    arguments << cadfich;
    myProcess->start(cadexec,arguments);

    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return 2;
       }
    if (!myProcess->waitForFinished (60000))
       {
         delete myProcess;
         return 2;
       }
*/
    QApplication::processEvents();

    arguments.clear();
    cadexec=programa_imprimir();
    QString qfich;
    qfich=dirtrabajo();
    qfich.append(QDir::separator());
    qfich+=fichero;
    qfich+=".pdf";
/*    if (programa_imprimir()=="kprinter")
        {
         arguments << "--caption";
         arguments << "KEME-Contabilidad";
         cadarg=dirtrabajo();
         cadarg.append(QDir::separator());
         cadarg+=fichero;
         cadarg+=".ps";
         arguments << adapta(cadarg);
         // arguments << "&";
         myProcess->start(cadexec,arguments);
         if (!myProcess->waitForStarted ())
           {
            delete myProcess;
            return 3;
           }
         myProcess->waitForFinished(-1);
         delete myProcess;
         return 0;
        }
     else */
    QApplication::processEvents();

    QString rpdf=rutacopiapdf();
     if (rpdf.isEmpty() || pref_forzar_ver_pdf())
        {
          cadexec+=" ";
          cadexec+=qfich;
          if (! myProcess->startDetached(cadexec))
            {
             delete myProcess;
             return 3;
            }
        }

     QApplication::processEvents();

    if (!rpdf.isEmpty())
       {
        QFile fich(qfich);
        QString destino=rpdf;
        destino.append(QDir::separator());
        destino.append(fichero);
        destino.append(cad_tiempo());
        destino.append(".pdf");
        if (eswindows()) fich.copy(QFile::encodeName(destino));
           else fich.copy(destino);

       }

    QApplication::processEvents();

  delete myProcess;
  return 0;
}


void desanulavencimiento(QString vto)
{
    basedatos::instancia()->updateVencimientosanuladonum(vto,false);
}


int existeclaveiva(QString cadena,QString *qtipo,QString *qre,QString *qdescrip)
{
    return basedatos::instancia()->selectTodoTiposivaclave(cadena,qtipo,qre,qdescrip);
}

void guardacambiostiposiva(QString clave,QString tipo,QString re,QString descripcion)
{
    basedatos::instancia()->updateTiposivatiporedescripcionclave(clave,tipo,re,descripcion);
}

void insertatipoiva(QString clave,QString tipo, QString re, QString descripcion)
{
    basedatos::instancia()->insertTiposiva(clave,tipo,re,descripcion);
}

void eliminatipoiva(QString qclave)
{
    basedatos::instancia()->deleteTiposivaclave(qclave);
}

/* QString nifcomunitariocta(QString codigo)
{
    return basedatos::instancia()->selectNifcomunitariodatossubcuentacuenta(codigo);
}
*/

QString cifcta(QString codigo)
{
    return basedatos::instancia()->selectCifdatossubcuentacuenta(codigo);
}


QString ruta_java()
{
  // return "qrc:/java/java";

  if (eswindows())
    {
      QString ruta;
      //ruta=windirprogram();
      //return adapta( ruta+"\\keme4\\java");
      ruta=trayconfig();
      return adapta (ruta+"\\report");

    }
  QString ruta=trayconfig();
  ruta+="/report";
  return ruta;// "/usr/share/java/keme";
}

void informe_jasper_xml(QString ficherojasper, QString ficheroxml,
                        QString expresionxpath, QString fichpdf,
                        QString ruta_graf)
{

    // QString cadexec="java -jar /usr/share/java/keme/jasp_xml2pdf.jar ";

    QString cadexec="java"; // /usr/share/java/keme/jasp_xml2pdf.jar ";
    QStringList args;
    args << "-jar";
    args << ruta_java()+QDir::separator()+"jasp_xml2pdf.jar";

    // QMessageBox::warning( 0, QObject::tr("Generar informe"),ruta_java());

    QObject *parent=NULL;

   if (eswindows())
       {
        ficherojasper=adapta(ficherojasper);
        ficheroxml=adapta(ficheroxml);
        fichpdf=adapta(fichpdf);
        ruta_graf=adapta(ruta_graf);
       }
/*     cadexec+=ficherojasper;
     cadexec+=" ";
     cadexec+=ficheroxml;
     cadexec+=" ";
     cadexec+= expresionxpath;
     cadexec+=" ";
     cadexec+= fichpdf;
     cadexec+=" ";
     cadexec+=ruta_graf;*/

    args << ficherojasper;
    args << ficheroxml;
    args << expresionxpath;
    args << fichpdf;
    args << ruta_graf;

    // QMessageBox::information( 0, QObject::tr("Generar informe"),cadexec + " -jar " +
    //                          ruta_java()+QDir::separator()+"jasp_xml2pdf.jar "+
    //                      ficherojasper + " " + ficheroxml + " " + expresionxpath + " "
    //                      + fichpdf + " "+ ruta_graf);

     QProcess *myProcess = new QProcess(parent);

     // myProcess->setWorkingDirectory(adapta(dirtrabajo()));

     myProcess->start(cadexec,args);

    // system(cadexec);
    if (!myProcess->waitForStarted ())
       {
         QMessageBox::warning( 0, QObject::tr("Generar informe"),
                               QObject::tr("Problemas procesando archivo XML"));
         delete myProcess;
         return ;
       }
    if (!myProcess->waitForFinished (300000))
       {
        QMessageBox::warning( 0, QObject::tr("Generar informe"),
                              QObject::tr("Problemas procesando archivo XML"));
         delete myProcess;
         return ;
       }

    delete myProcess;

    QString rpdf=rutacopiapdf();
    QStringList arguments; arguments<<fichpdf;
   if (rpdf.isEmpty() || pref_forzar_ver_pdf())
     {
       if (WINDOWS)
         {
           #ifdef _WIN32
           QT_WA({

           ShellExecute(0, 0, (TCHAR *)arguments.at(0).utf16(), 0, 0, SW_SHOWNORMAL);

           } , {

           ShellExecuteA(0, 0, arguments.at(0).toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL);

           });
           #endif
           #ifdef _WIN64
           QT_WA({

           ShellExecute(0, 0, (TCHAR *)arguments.at(0).utf16(), 0, 0, SW_SHOWNORMAL);

           } , {

           ShellExecuteA(0, 0, arguments.at(0).toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL);

           });
           #endif
         }
       else
        {
         if (!ejecuta(aplicacionabrirfich(extensionfich(fichpdf)),fichpdf))
          QMessageBox::warning( 0, QObject::tr("FACTURA EN PDF"),
                              QObject::tr("No se puede abrir ")+fichpdf+QObject::tr(" con ")+
                              aplicacionabrirfich(extensionfich(fichpdf)) + "\n" +
                              QObject::tr("Verifique el diálogo de preferencias"));
        }
      }
    if (!rpdf.isEmpty())
           { // vamos a copiar el pdf al destino
            QFile fich(fichpdf);
            QString destino=rpdf;
            destino.append(QDir::separator());
            QString nombrefich=fichpdf.mid(fichpdf.lastIndexOf(QDir::separator())+1);
            destino.append(nombrefich);
            destino.remove(".pdf");
            destino.append(cad_tiempo());
            destino.append(".pdf");
            // destino.append(".pdf");
            if (eswindows()) fich.copy(QFile::encodeName(destino));
              else fich.copy(destino);

         }
}


int consultalatex(QString qfichero)
{
    QProgressDialog progreso(QObject::tr("Generando informe ... ")
                             , 0, 0, 0);
    progreso.setWindowModality(Qt::WindowModal);
    progreso.setMinimumDuration ( 0 );
    progreso.setWindowTitle(QObject::tr("Generando  ... "));;
    progreso.show();
    progreso.update();
    QApplication::processEvents();


    QString pasafich=dirtrabajo();
    pasafich.append(QDir::separator());
    pasafich.append(qfichero);
    pasafich.append(".tex");
    if (filtrartexaxtivado()) filtratex(pasafich);

    QString fichero=qfichero;

    QString cadexec;
    // if (WINDOWS) cadexec="latex";
    //    else cadexec="pdflatex";
    cadexec="pdflatex";
    QString cadarg=dirtrabajo();
    cadarg.append(QDir::separator());
    cadarg+=fichero;

    QObject *parent=NULL;

     QStringList arguments;
     arguments << adapta(cadarg);

     QProcess *myProcess = new QProcess(parent);

     myProcess-> setWorkingDirectory(adapta(dirtrabajo()));

     QApplication::processEvents();

     myProcess->start(cadexec,arguments);

    // system(cadexec);
    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return 1;
       }
    QApplication::processEvents();
    if (!myProcess->waitForFinished (60000))
       {
         delete myProcess;
         return 1;
       }


    QApplication::processEvents();

    // ---------------------------------------------------------------
    // borramos aux y log
    QString fich_aux=cadarg+".aux";
    QString fich_log=cadarg+".log";

    QFile faux(eswindows() ? QFile::encodeName(fich_aux): fich_aux);
    faux.remove();
    QFile flog(eswindows() ? QFile::encodeName(fich_log) : fich_log);
    flog.remove();
    // ---------------------------------------------------------------

    QString rpdf=rutacopiapdf();
    /*
    if (eswindows() && !rpdf.isEmpty())
       {
        QString cadexec2="pdf";
        cadexec2+=cadexec;
        myProcess->start(cadexec2,arguments);
        if (!myProcess->waitForStarted ())
           {
             delete myProcess;
             return 1;
           }
        if (!myProcess->waitForFinished (60000))
           {
             delete myProcess;
             return 1;
           }

       }*/

    arguments.clear();

    QApplication::processEvents();

    cadexec=visordvi();
    if (WINDOWS)cadexec="start";
    cadarg=dirtrabajo();
    cadarg.append(QDir::separator());
    cadarg+=fichero;
    QString cadfichpdf=cadarg+".pdf";
    /*if (WINDOWS) cadarg+=".dvi";
       else cadarg+=".pdf";*/
    cadarg+=".pdf";
    arguments << adapta(cadarg);
    // arguments << "&";

    QApplication::processEvents();

    if (rpdf.isEmpty() || pref_forzar_ver_pdf())
     {
        if (WINDOWS)
          {
            #ifdef _WIN32
            QT_WA({

            ShellExecute(0, 0, (TCHAR *)arguments.at(0).utf16(), 0, 0, SW_SHOWNORMAL);

            } , {

            ShellExecuteA(0, 0, arguments.at(0).toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL);

            });
            #endif
            #ifdef _WIN64
            QT_WA({

            ShellExecute(0, 0, (TCHAR *)arguments.at(0).utf16(), 0, 0, SW_SHOWNORMAL);

            } , {

            ShellExecuteA(0, 0, arguments.at(0).toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL);

            });
            #endif
          }
        else
         {
           if (! myProcess->startDetached(cadexec,arguments)) // cadexec,arguments
            {
             delete myProcess;
             return 2;
            }
         }
     }

    delete myProcess;

    QApplication::processEvents();

    if (!rpdf.isEmpty())        
       {
        //
        QFile fich(cadfichpdf);
        QString destino=rpdf;
        destino.append(QDir::separator());
        destino.append(fichero);
        destino.append(cad_tiempo());
        destino.append(".pdf");
        if (eswindows()) fich.copy(QFile::encodeName(destino));
           else fich.copy(destino);

       }

    QApplication::processEvents();

  return 0;
}




int consultalatex2(QString qfichero)
{
    QProgressDialog progreso(QObject::tr("Generando informe ... ")
                             , 0, 0, 0);
    progreso.setWindowModality(Qt::WindowModal);
    progreso.setMinimumDuration ( 0 );
    progreso.setWindowTitle(QObject::tr("Generando  ... "));;
    progreso.show();
    progreso.update();
    QApplication::processEvents();

    QString pasafich=dirtrabajo();
    pasafich.append(QDir::separator());
    pasafich.append(qfichero);
    pasafich.append(".tex");
    if (filtrartexaxtivado()) filtratex(pasafich);
    QApplication::processEvents();

    QString fichero=qfichero;

    QString cadexec;
    /*if (WINDOWS) cadexec="latex";
       else cadexec="pdflatex";*/
    cadexec="pdflatex";
    QString cadarg=dirtrabajo();
    cadarg.append(QDir::separator());
    cadarg+=fichero;

    QObject *parent=NULL;

     QStringList arguments;
     arguments << adapta(cadarg);

     QProcess *myProcess = new QProcess(parent);

     myProcess-> setWorkingDirectory(dirtrabajo());

     QApplication::processEvents();

     myProcess->start(cadexec,arguments);

    // system(cadexec);
    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return 1;
       }
    QApplication::processEvents();
    if (!myProcess->waitForFinished ())
       {
         delete myProcess;
         return 1;
       }

    QApplication::processEvents();

     myProcess->start(cadexec,arguments);

    // system(cadexec);
    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return 1;
       }
    QApplication::processEvents();
    if (!myProcess->waitForFinished ())
       {
         delete myProcess;
         return 1;
       }

    QApplication::processEvents();

    // si es Windows,  se ha generado el pdf


    // ---------------------------------------------------------------
    // borramos aux y log
    QString fich_aux=cadarg+".aux";
    QString fich_log=cadarg+".log";

    QFile faux(eswindows() ? QFile::encodeName(fich_aux): fich_aux);
    faux.remove();
    QFile flog(eswindows() ? QFile::encodeName(fich_log) : fich_log);
    flog.remove();
    // ---------------------------------------------------------------


    QString rpdf=rutacopiapdf();
    /*
    if (eswindows() && !rpdf.isEmpty())
       {
        QString cadexec2="pdf";
        cadexec2+=cadexec;
        myProcess->start(cadexec2,arguments);
        if (!myProcess->waitForStarted ())
           {
             delete myProcess;
             return 1;
           }
        if (!myProcess->waitForFinished (60000))
           {
             delete myProcess;
             return 1;
           }

       }
     */
    QApplication::processEvents();

    arguments.clear();
    cadexec=visordvi();
    if (WINDOWS) cadexec="start";

    cadarg=dirtrabajo();
    cadarg.append(QDir::separator());
    cadarg+=fichero;
    QString cadfichpdf=cadarg+".pdf";
    /*if (WINDOWS) cadarg+=".dvi";
       else cadarg+=".pdf";*/
    cadarg+=".pdf";
    arguments << adapta(cadarg);
    // arguments << "&";

    QApplication::processEvents();

    if (rpdf.isEmpty() || pref_forzar_ver_pdf())
      {
        if (WINDOWS)
          {
            #ifdef _WIN32
            QT_WA({

            ShellExecute(0, 0, (TCHAR *)arguments.at(0).utf16(), 0, 0, SW_SHOWNORMAL);

            } , {

            ShellExecuteA(0, 0, arguments.at(0).toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL);

            });
            #endif
            #ifdef _WIN64
            QT_WA({

            ShellExecute(0, 0, (TCHAR *)arguments.at(0).utf16(), 0, 0, SW_SHOWNORMAL);

            } , {

            ShellExecuteA(0, 0, arguments.at(0).toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL);

            });
            #endif
          }
        else
         {
           if (! myProcess->startDetached(cadexec,arguments)) // cadexec,arguments
            {
             delete myProcess;
             return 2;
            }
         }
      }

    delete myProcess;

    QApplication::processEvents();

    if (!rpdf.isEmpty())
       {
        //
        QFile fich(cadfichpdf);
        QString destino=rpdf;
        destino.append(QDir::separator());
        destino.append(fichero);
        destino.append(cad_tiempo());
        destino.append(".pdf");
        if (eswindows()) fich.copy(QFile::encodeName(destino));
           else fich.copy(destino);

       }

    QApplication::processEvents();

  return 0;
}


int consultalatex2fichabs(QString qfichero)
{
    QProgressDialog progreso(QObject::tr("Generando informe ... ")
                             , 0, 0, 0);
    progreso.setWindowModality(Qt::WindowModal);
    progreso.setMinimumDuration ( 0 );
    progreso.setWindowTitle(QObject::tr("Generando  ... "));;
    progreso.show();
    progreso.update();
    QApplication::processEvents();

    if (WINDOWS && !qfichero.contains(QDir::separator()))
        qfichero.replace('/',QDir::separator());
    if (filtrartexaxtivado()) filtratex(qfichero);

    QString fichero=qfichero;
    fichero.truncate(qfichero.length()-4);
    QString cadarg=fichero;

    QObject *parent=NULL;

     QStringList arguments;
     arguments << adapta(cadarg);

     QString cadexec;
     /*if (WINDOWS) cadexec="latex";
        else cadexec="pdflatex";*/
     cadexec="pdflatex";
     QProcess *myProcess = new QProcess(parent);

     QApplication::processEvents();

     QString dirtrab=qfichero.left(qfichero.lastIndexOf(QDir::separator()));
     QString nombrefich=qfichero.mid(qfichero.lastIndexOf(QDir::separator())+1);
     nombrefich.remove(".tex");

     myProcess-> setWorkingDirectory(adapta(dirtrab));

     myProcess->start(cadexec,arguments);

     QApplication::processEvents();

    // system(cadexec);
    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return 1;
       }
    QApplication::processEvents();
    if (!myProcess->waitForFinished ())
       {
         delete myProcess;
         return 1;
       }

     myProcess->start(cadexec,arguments);

    // system(cadexec);
    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return 1;
       }
    QApplication::processEvents();
    if (!myProcess->waitForFinished ())
       {
         delete myProcess;
         return 1;
       }

    // si es Windows,  se ha generado el pdf
    QApplication::processEvents();


    // ---------------------------------------------------------------
    // borramos aux y log
    QString fich_aux=cadarg+".aux";
    QString fich_log=cadarg+".log";

    QFile faux(eswindows() ? QFile::encodeName(fich_aux): fich_aux);
    faux.remove();
    QFile flog(eswindows() ? QFile::encodeName(fich_log) : fich_log);
    flog.remove();
    // ---------------------------------------------------------------


    QString rpdf=rutacopiapdf();
    /*if (eswindows() && !rpdf.isEmpty())
       {
        QString cadexec2="pdf";
        cadexec2+=cadexec;
        myProcess->start(cadexec2,arguments);
        if (!myProcess->waitForStarted ())
           {
             delete myProcess;
             return 1;
           }
        if (!myProcess->waitForFinished (60000))
           {
             delete myProcess;
             return 1;
           }

       }*/

    // si es Windows, se ha generado el pdf (segunda pasada)

/*    if (eswindows() && !rpdf.isEmpty())
       {
        QString cadexec2="pdf";
        cadexec2+=cadexec;
        myProcess->start(cadexec2,arguments);
        if (!myProcess->waitForStarted ())
           {
             delete myProcess;
             return 1;
           }
        if (!myProcess->waitForFinished (60000))
           {
             delete myProcess;
             return 1;
           }

       } */


    arguments.clear();
    cadexec=visordvi();
    if (WINDOWS) cadexec="start";
    cadarg=fichero;

    QString cadfichpdf=cadarg+".pdf";
    /* if (WINDOWS) cadarg+=".dvi";
       else cadarg+=".pdf";*/
    cadarg+=".pdf";

    arguments << adapta(cadarg);

    QApplication::processEvents();

    if (rpdf.isEmpty() || pref_forzar_ver_pdf())
      {
        if (WINDOWS)
          {
            #ifdef _WIN32
            QT_WA({

            ShellExecute(0, 0, (TCHAR *)arguments.at(0).utf16(), 0, 0, SW_SHOWNORMAL);

            } , {

            ShellExecuteA(0, 0, arguments.at(0).toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL);

            });
            #endif
            #ifdef _WIN64
            QT_WA({

            ShellExecute(0, 0, (TCHAR *)arguments.at(0).utf16(), 0, 0, SW_SHOWNORMAL);

            } , {

            ShellExecuteA(0, 0, arguments.at(0).toLocal8Bit().constData(), 0, 0, SW_SHOWNORMAL);

            });
            #endif
          }
        else
         {
           if (! myProcess->startDetached(cadexec,arguments)) // cadexec,arguments
            {
             delete myProcess;
             return 2;
            }
         }
      }

    delete myProcess;

    QApplication::processEvents();

    if (!rpdf.isEmpty())
       {
        //
        QFile fich(cadfichpdf);
        QString destino=rpdf;
        destino.append(QDir::separator());
        destino.append(nombrefich);
        destino.append(cad_tiempo());
        destino.append(".pdf");
        if (eswindows()) fich.copy(QFile::encodeName(destino));
           else fich.copy(destino);

       }

    QApplication::processEvents();

  return 0;
}



void borraestado(QString estado)
{
    basedatos::instancia()->deleteEstadostitulo(estado);
    basedatos::instancia()->deleteCabeceraestadostitulo(estado);
}

void desbloqueasaldosmedios()
{
    basedatos::instancia()->updateConfiguracionbloqsaldosmedios();
}

bool exportarestado(QString estado, QString nombre)
{
  // ----------------------------------------------------------------------------------
 QDomDocument doc("EstadoContable");
 QDomElement root = doc.createElement("EstadoContable");
 doc.appendChild(root);

 QDomElement tag = doc.createElement("Heading");
 root.appendChild(tag);

  // titulo,cabecera,parte1,parte2, observaciones,formulabasepor,'2000-01-01', "
  //      "ejercicio1,ejercicio2,diarios,estadosmedios,"
  //      "valorbasepor1, valorbasepor2,analitica,haycolref,colref,ci,grafico

  QSqlQuery query = basedatos::instancia()->selectTodoespecialcabeceraestadostitulo(estado);

   if ( query.isActive() )
       while (query.next())
          {
            QDomElement tagtit = doc.createElement("Titulo");
            tag.appendChild(tagtit);
            QDomText texttit = doc.createTextNode(filtracadxml(query.value(0).toString()));
            tagtit.appendChild(texttit);

            QDomElement tagcabecera = doc.createElement("Cabecera");
            tag.appendChild(tagcabecera);
            QDomText textcabecera = doc.createTextNode(filtracadxml(query.value(1).toString()));
            tagcabecera.appendChild(textcabecera);

            QDomElement tagparte1 = doc.createElement("Parte1");
            tag.appendChild(tagparte1);
            QDomText textparte1 = doc.createTextNode(filtracadxml(query.value(2).toString()));
            tagparte1.appendChild(textparte1);

            QDomElement tagparte2 = doc.createElement("Parte2");
            tag.appendChild(tagparte2);
            QDomText textparte2 = doc.createTextNode(filtracadxml(query.value(3).toString()));
            tagparte2.appendChild(textparte2);

            QDomElement tagobservaciones = doc.createElement("Observaciones");
            tag.appendChild(tagobservaciones);
            QDomText textobservaciones = doc.createTextNode(filtracadxml(query.value(4).toString()));
            tagobservaciones.appendChild(textobservaciones);

            QDomElement tagformulabasepor = doc.createElement("FormulaBase");
            tag.appendChild(tagformulabasepor);
            QDomText textformulabasepor = doc.createTextNode(filtracadxml(query.value(5).toString()));
            tagformulabasepor.appendChild(textformulabasepor);

            QDomElement tagejercicio1 = doc.createElement("Ejercicio1");
            tag.appendChild(tagejercicio1);
            QDomText textejercicio1 = doc.createTextNode(filtracadxml(query.value(7).toString()));
            tagejercicio1.appendChild(textejercicio1);

            QDomElement tagejercicio2 = doc.createElement("Ejercicio2");
            tag.appendChild(tagejercicio2);
            QDomText textejercicio2 = doc.createTextNode(filtracadxml(query.value(8).toString()));
            tagejercicio2.appendChild(textejercicio2);

            QDomElement tagdiarios = doc.createElement("Diarios");
            tag.appendChild(tagdiarios);
            QDomText textdiarios = doc.createTextNode(filtracadxml(query.value(9).toString()));
            tagdiarios.appendChild(textdiarios);

            QDomElement tagestadosmedios = doc.createElement("EstadosMedios");
            tag.appendChild(tagestadosmedios);
            QDomText textestadosmedios = doc.createTextNode(query.value(10).toBool() ? "true" : "false");
            tagestadosmedios.appendChild(textestadosmedios);

            QDomElement tagvalorbasepor1 = doc.createElement("valorbasepor1");
            tag.appendChild(tagvalorbasepor1);
            QDomText textvalorbasepor1 = doc.createTextNode(filtracadxml(query.value(11).toString()));
            tagvalorbasepor1.appendChild(textvalorbasepor1);

            QDomElement tagvalorbasepor2 = doc.createElement("valorbasepor2");
            tag.appendChild(tagvalorbasepor2);
            QDomText textvalorbasepor2 = doc.createTextNode(filtracadxml(query.value(12).toString()));
            tagvalorbasepor2.appendChild(textvalorbasepor2);

            QDomElement taganalitica = doc.createElement("analitica");
            tag.appendChild(taganalitica);
            QDomText textanalitica = doc.createTextNode(query.value(13).toBool() ? "true" : "false");
            taganalitica.appendChild(textanalitica);

            QDomElement taghaycolref = doc.createElement("haycolref");
            tag.appendChild(taghaycolref);
            QDomText texthaycolref = doc.createTextNode(query.value(14).toBool() ? "true" : "false");
            taghaycolref.appendChild(texthaycolref);

            QDomElement tagcolref = doc.createElement("colref");
            tag.appendChild(tagcolref);
            QDomText textcolref = doc.createTextNode(filtracadxml(query.value(15).toString()));
            tagcolref.appendChild(textcolref);

            QDomElement tagci = doc.createElement("ci");
            tag.appendChild(tagci);
            QDomText textci = doc.createTextNode(filtracadxml(query.value(16).toString()));
            tagci.appendChild(textci);

            QDomElement taggrafico = doc.createElement("grafico");
            tag.appendChild(taggrafico);
            QDomText textgrafico = doc.createTextNode(query.value(17).toBool() ? "true" : "false");
            taggrafico.appendChild(textgrafico);

            QDomElement tagdesglose = doc.createElement("desglose");
            tag.appendChild(tagdesglose);
            QDomText textdesglose = doc.createTextNode(query.value(18).toBool() ? "true" : "false");
            tagdesglose.appendChild(textdesglose);

            QDomElement tagdesglose_ctas = doc.createElement("desglose_ctas");
            tag.appendChild(tagdesglose_ctas);
            QDomText textdesglose_ctas = doc.createTextNode(query.value(19).toBool() ? "true" : "false");
            tagdesglose_ctas.appendChild(textdesglose_ctas);

            QDomElement tagdescabecera = doc.createElement("descabecera");
            tag.appendChild(tagdescabecera);
            QDomText textdescabecera = doc.createTextNode(filtracadxml(query.value(20).toString()));
            tagdescabecera.appendChild(textdescabecera);

            QDomElement tagdespie = doc.createElement("despie");
            tag.appendChild(tagdespie);
            QDomText textdespie = doc.createTextNode(filtracadxml(query.value(21).toString()));
            tagdespie.appendChild(textdespie);
         }

 QDomElement tag3 = doc.createElement("Cuerpo");
 root.appendChild(tag3);

   // titulo,apartado,parte1,clave,nodo,formula
   query = basedatos::instancia()->selectTituloapartadoparte1clavenodoformulaestadostitulo(estado);
   if ( query.isActive() )
       while (query.next())
          {
              QDomElement tag4 = doc.createElement("Registro");
              tag3.appendChild(tag4);

              QDomElement tagtitulo = doc.createElement("titulocod");
              tag4.appendChild(tagtitulo);
              QDomText texttitulo = doc.createTextNode(filtracadxml(query.value(0).toString()));
              tagtitulo.appendChild(texttitulo);

              QDomElement tagapartado = doc.createElement("apartado");
              tag4.appendChild(tagapartado);
              QDomText textapartado = doc.createTextNode(filtracadxml(query.value(1).toString()));
              tagapartado.appendChild(textapartado);

              QDomElement tagparte1 = doc.createElement("parte1");
              tag4.appendChild(tagparte1);
              QDomText textparte1 = doc.createTextNode(query.value(2).toBool() ? "true" : "false");
              tagparte1.appendChild(textparte1);

              QDomElement tagclave = doc.createElement("clave");
              tag4.appendChild(tagclave);
              QDomText textclave = doc.createTextNode(filtracadxml(query.value(3).toString()));
              tagclave.appendChild(textclave);

              QDomElement tagnode = doc.createElement("nodo");
              tag4.appendChild(tagnode);
              QDomText textnode = doc.createTextNode(filtracadxml(query.value(4).toString()));
              tagnode.appendChild(textnode);

              QDomElement tagformula = doc.createElement("formula");
              tag4.appendChild(tagformula);
              QDomText textformula = doc.createTextNode(filtracadxml(query.value(5).toString()));
              tagformula.appendChild(textformula);
         }


 QString xml = doc.toString();
  // -----------------------------------------------------------------------------------

  QFile fichero( adapta(nombre)  );

  if ( !fichero.open( QIODevice::WriteOnly ) ) return false;

  QTextStream stream( &fichero );
  stream.setCodec("UTF-8");

  stream << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
  stream << "<?xml-stylesheet type=\"text/xsl\" href=\"estado2html.xslt\"?>";

  // bool esmysql = ( basedatos::instancia()->cualControlador() == basedatos::MYSQL );

  // ------------------------------------------------------------------------------------
  stream << xml;
  fichero.close();

           QString fichxslt=dirtrabajobd();
           fichxslt.append(QDir::separator());
           fichxslt.append("estado2html.xslt");
           if (!QFile::exists(fichxslt))
              {
               QString pasa2;
               pasa2=traydoc();
               pasa2.append(QDir::separator());
               pasa2.append("estado2html.xslt");
               if (eswindows()) pasa2=QFile::encodeName(pasa2);
               QFile fichero(pasa2);
               if (eswindows()) fichero.copy(QFile::encodeName(fichxslt));
                  else fichero.copy(fichxslt);
              }

  return true;
  // ------------------------------------------------------------------------------------
}


int importarestado(QString nombre)
{
  QDomDocument doc("EstadoContable");
  QFile fichero(adapta(nombre));
  if (!fichero.exists()) return false;

  if ( !fichero.open( QIODevice::ReadOnly ) ) return false;

  if (!doc.setContent(&fichero)) {
     fichero.close();
     return false;
    }
 fichero.close();

 QString titulo=etiquetadenodo(doc,"Titulo");

 // QMessageBox::warning( 0, QObject::tr("importar"),titulo);

  //buscamos el nombre
  if ( basedatos::instancia()->sihaytitulocabeceraestados(titulo) == 2 ) { return 2; }

            QString cabecera,parte1,parte2, observaciones,formulabasepor,fechacalculo,
                     ejercicio1,ejercicio2,diarios,estadosmedios,
                     valorbasepor1, valorbasepor2,analitica,haycolref,colref,ci,grafico,
                     desglose,desglose_ctas,descabecera,despie;
            cabecera=etiquetadenodo(doc,"Cabecera");
            parte1=etiquetadenodo(doc,"Parte1");
            parte2=etiquetadenodo(doc,"Parte2");
            observaciones=etiquetadenodo(doc,"Observaciones");
            formulabasepor=etiquetadenodo(doc,"FormulaBase");
            fechacalculo="2000-01-01";
            ejercicio1=etiquetadenodo(doc,"Ejercicio1");
            ejercicio2=etiquetadenodo(doc,"Ejercicio2");
            diarios=etiquetadenodo(doc,"Diarios");
            estadosmedios=etiquetadenodo(doc,"EstadosMedios");
            valorbasepor1=etiquetadenodo(doc,"valorbasepor1");
            valorbasepor2=etiquetadenodo(doc,"valorbasepor2");
            analitica=etiquetadenodo(doc,"analitica");
            haycolref=etiquetadenodo(doc,"haycolref");
            colref=etiquetadenodo(doc,"colref");
            ci=etiquetadenodo(doc,"ci");
            grafico=etiquetadenodo(doc,"grafico");
            if (grafico.length()==0) grafico="false";
            desglose=etiquetadenodo(doc,"desglose");
            desglose_ctas=etiquetadenodo(doc,"desglose_ctas");
            if (desglose_ctas.isEmpty()) desglose_ctas="false";
            descabecera=etiquetadenodo(doc,"descabecera");
            despie=etiquetadenodo(doc,"despie");
            // QMessageBox::warning( 0, QObject::tr("importar"),desglose_ctas);

            basedatos::instancia()->insert18Cabeceraestados(titulo,cabecera,parte1,parte2,
                observaciones,formulabasepor,fechacalculo,ejercicio1, 
                ejercicio2,diarios,estadosmedios,valorbasepor1,valorbasepor2, 
                analitica,haycolref,colref,ci,grafico,desglose,desglose_ctas,descabecera,despie);

   // importamos el resto de fichero
   // buscamo la etiqueta cuerpo
 QDomNodeList listanodos=doc.elementsByTagName ("Cuerpo");
 if (listanodos.size()==0) return false;
 QDomNode nodo= listanodos.at(0);
 QDomNode n=nodo.firstChild(); //registro

  while(!n.isNull()) {
     QDomElement e = n.toElement(); // try to convert the node to an element.
     QDomNode nn=e.firstChild();
     QString apartado,lparte1,clave,nodo,formula,importe1,importe2,calculado,referencia;
     while (!nn.isNull())
          {
            QDomElement ee = nn.toElement();
            if(!ee.isNull()) {
              if (ee.tagName()=="apartado")
                 {
                  QDomNode nnn=ee.firstChild();
                  QDomText t = nnn.toText();
                  apartado= t.data();
                 }
              if (ee.tagName()=="parte1")
                 {
                  QDomNode nnn=ee.firstChild();
                  QDomText t = nnn.toText();
                  lparte1= t.data();
                 }
              if (ee.tagName()=="clave")
                 {
                  QDomNode nnn=ee.firstChild();
                  QDomText t = nnn.toText();
                  clave= t.data();
                 }
              if (ee.tagName()=="nodo")
                 {
                  QDomNode nnn=ee.firstChild();
                  QDomText t = nnn.toText();
                  nodo= t.data();
                 }
              if (ee.tagName()=="formula")
                 {
                  QDomNode nnn=ee.firstChild();
                  QDomText t = nnn.toText();
                  formula= t.data();
                 }
              }
            nn=nn.nextSibling();
           }

     importe1="0";
     importe2="0";
     calculado="false";
     referencia="";
     basedatos::instancia()->insertEstados(titulo,apartado,lparte1,clave,nodo,
                formula,importe1,importe2,calculado,referencia);
     n=n.nextSibling();
  }
 return 1;
}

QString etiquetadenodo(QDomDocument doc,QString etiqueta)
{
 QDomNodeList listanodos=doc.elementsByTagName (etiqueta);
 if (listanodos.size()==0) return "";
 QDomNode nodo= listanodos.at(0);
 QDomNode nn=nodo.firstChild();
 QDomText t = nn.toText();
 return t.data();
}


QString textodenodo(QDomNode nodo)
{
 QDomNode nn=nodo.firstChild();
 QDomText t = nn.toText();
 return t.data();
}


int generapdffichabs(QString qfichero)
{
    if (filtrartexaxtivado()) filtratex(qfichero);

    QString fichero=qfichero;
    fichero.truncate(qfichero.length()-4);
    QString cadarg=fichero;

    QObject *parent=NULL;

     QStringList arguments;
     arguments << adapta(cadarg);

     QString cadexec="latex";
     QProcess *myProcess = new QProcess(parent);

     QApplication::processEvents();
     QString qdirtrabajo=qfichero.left(qfichero.lastIndexOf(QDir::separator())+1);
     myProcess-> setWorkingDirectory(adapta(qdirtrabajo));

     myProcess->start(cadexec,arguments);

     QApplication::processEvents();

    // system(cadexec);
     myProcess->start(cadexec,arguments);

    // system(cadexec);
    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return 1;
       }

    if (!myProcess->waitForFinished ())
       {
         delete myProcess;
         return 1;
       }

     myProcess->start(cadexec,arguments);

     QApplication::processEvents();

    // system(cadexec);
    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return 1;
       }
    if (!myProcess->waitForFinished ())
       {
         delete myProcess;
         return 1;
       }


    arguments.clear();
    cadexec="dvipdf";
    if (eswindows()) cadexec="dvipdfm";

    cadarg=adapta(fichero);
    cadarg+=".dvi";
    arguments << cadarg;
    // arguments << "&";

    myProcess->start(cadexec,arguments);

    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return 2;
       }

    if (!myProcess->waitForFinished ())
       {
         delete myProcess;
         return 2;
       }

  delete myProcess;
  QFile::remove(fichero+".dvi");
  QFile::remove(fichero+".aux");
  QFile::remove(fichero+".log");
  return 0;
}


bool int_asientos_ej_cerrado(QString asientoinicial,QString asientofinal)
{
    return basedatos::instancia()->int_asientos_ej_cerrado(asientoinicial,asientofinal);
}

void pasasaldo(QString cuenta,QString importe,bool cargo)
{
    basedatos::instancia()->updateSaldossubcuentasaldocodigo(cuenta,importe,cargo);
}

bool cuentaenlibroiva(QString cuenta)
{
    return basedatos::instancia()->cuentaenlibroiva(cuenta);
}

bool paseapertura(QString pase)
{
    return basedatos::instancia()->paseapertura(pase);
}

bool paseconvto(QString pase)
{
    return basedatos::instancia()->paseconvto(pase);
}

bool pasepagovto(QString pase)
{
    return basedatos::instancia()->pasepagovto(pase);
}

bool paseenlibroivaeib(QString pase)
{
    return basedatos::instancia()->paseenlibroivaeib(pase);
}


void actualizasaldos()
{
    basedatos::instancia()->actucodigos_saldossubcuenta();
    basedatos::instancia()->updateSaldossubcuentasaldo0();
    QSqlQuery query = basedatos::instancia()->selectCodigosaldossubcuenta();
    if ( query.isActive() )
    {
        while (query.next())
        {
            double saldo = saldocuentaendiario(query.value(0).toString());
	        QString cadnum;
	        cadnum.setNum(saldo,'f',2);
            basedatos::instancia()->updateSaldossubcuentasaldocodigo( query.value(0).toString() , cadnum , true);
        }
    }
}


QString versionbd()
{
    return basedatos::instancia()->selectVersionconfiguracion();
}



QString traydoc(void)
{
 QString bundle;
#ifdef Q_WS_MAC
    CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef,
                                           kCFURLPOSIXPathStyle);
    const char *pathPtr = CFStringGetCStringPtr(macPath,
                                           CFStringGetSystemEncoding());
    bundle=bundle.fromLocal8Bit(pathPtr) + "/Contents/Resources/doc";
    // qDebug("Path = %s", pathPtr);
    CFRelease(appUrlRef);
    CFRelease(macPath);
#endif
  if (es_os_x()) return bundle;

  if (!WINDOWS) return "/usr/share/doc/keme4";
   else
        {
         QString cadena=cadena.fromLocal8Bit(getenv("ProgramFiles"));
         cadena+="\\keme4\\doc";
         return cadena;
        }
}
 
QString traynombrefichayuda(void)
 {
     QString nombrefich;
     nombrefich=traydoc();
     if (WINDOWS) nombrefich+="\\manual\\index.html";
        else nombrefich+="/manual/index.html";
     return nombrefich;
 }


bool importanuevoplan(QString qfichero)
{
  QFile fichero(adapta(qfichero));
  if (!fichero.exists()) return false;
   if ( fichero.open( QIODevice::ReadOnly ) ) 
     {
        QTextStream stream( &fichero );
        stream.setCodec("UTF-8");
        QString linea;
         while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            QString codigo,descripcion;
            codigo=linea.section('\t',0,0);
            descripcion=linea.section('\t',1,1);
            basedatos::instancia()->insertNuevoplan(codigo,descripcion,false);
           }
         fichero.close();
      } else return false;
   return true;
}

void borraplannuevo()
{
    basedatos::instancia()->deleteNuevoplan();
}



int subgrupocongruponuevoplan(QString cadena)
{
    return basedatos::instancia()->subgrupocongruponuevoplan(cadena);
}


int cuentaconsubgruponuevoplan(QString cadena)
{
    return basedatos::instancia()->cuentaconsubgruponuevoplan(cadena);
}


int codigoconcuenta3digitosnuevoplan(QString cadena)
{
    return basedatos::instancia()->codigoconcuenta3digitosnuevoplan(cadena);
}

void guardacambiosplannuevo(QString codigo,QString descripcion)
{
    basedatos::instancia()->updateNuevoplandescripcioncodigo(codigo,descripcion);
}

int existecodigoplannuevo(QString cadena,QString *qdescrip)
{
    return basedatos::instancia()->existecodigoplannuevo(cadena,qdescrip);
}


void insertaenplannuevo(QString codigo,QString descripcion)
{
    basedatos::instancia()->insertNuevoplan(codigo,descripcion,false);
}

int itemplaneliminadonuevo(QString qcodigo)
{
    if ( basedatos::instancia()->siCodigoennuevoplanconlength(qcodigo) == 0 ) { return 0; }
    basedatos::instancia()->deleteNuevoplancodigo(qcodigo);
    return 1;
}


QString descripcioncuentanuevoplan(QString qcodigo)
{
    return basedatos::instancia()->selectDescripcionnuevoplancodigo(qcodigo);
}

bool codsubdivis(QString qcodigo)
{
    return basedatos::instancia()->codsubdivis(qcodigo,"plancontable");
}


bool codsubdivisplannuevo(QString qcodigo)
{
    return basedatos::instancia()->codsubdivis(qcodigo,"nuevoplan");
}


void cambiacuentaconta(QString origen, QString destino)
{
    basedatos::instancia()->updateAmortfiscalycontablecuenta_activocuenta_activo(origen,destino);
    basedatos::instancia()->updateAmortfiscalycontablecuenta_am_acumcuenta_am_acum(origen,destino);
    basedatos::instancia()->updataAmortfiscalycontablecuenta_gastocuenta_gasto(origen,destino);

    basedatos::instancia()->updateBorradorcuentacuenta(origen,destino);
    basedatos::instancia()->updateDatossubcuentacuentacuenta(origen,destino);
    basedatos::instancia()->updateDiariocuentacuenta(origen,destino);

    basedatos::instancia()->updateLibroivacta_base_ivacta_base_iva(origen,destino);
    basedatos::instancia()->updatelibroivacuenta_fracuenta_fra(origen,destino);

    basedatos::instancia()->updateplanamortizacionescuenta_activocuenta_activo(origen,destino);
    basedatos::instancia()->updatePlanamortizacionescuenta_am_acumcuenta_am_acum(origen,destino);
    basedatos::instancia()->updatePlanamortizacionescuenta_gastocuenta_gasto(origen,destino);

    basedatos::instancia()->updateAmortperscuentacuenta(origen,destino);
    basedatos::instancia()->updateSaldossubcuentacodigocodigo(origen,destino);
    basedatos::instancia()->updateVencimientoscta_ordenantecta_ordenante(origen,destino);
    basedatos::instancia()->updateVencimientoscta_tesoreriacta_tesoreria(origen,destino);
}


QString fsplash()
{
    QString fichsplash;
    if (!WINDOWS)
     {
      fichsplash="/usr/share/keme4";
      fichsplash+="/splash.png";
     }
      else 
           {
            fichsplash=fichsplash.fromLocal8Bit(getenv("ProgramFiles"));
            fichsplash+="\\keme4\\splash.png";
           }
    return fichsplash;
}

bool eswindows()
{
  return WINDOWS;
}

bool es_os_x()
{
  return OSX;
}


QString windirprogram()
{
 QString fichero=fichero.fromLocal8Bit(getenv("ProgramFiles"));
 return fichero;
}


bool exportarasmodelo(QString nombre,QString nfich)
{

 QDomDocument doc("AsientoAutomatico");
 QDomElement root = doc.createElement("AsientoAutomatico");
 doc.appendChild(root);

 // nombre y fecha
 QDomElement tag = doc.createElement("Heading");
 root.appendChild(tag);
 QSqlQuery query = basedatos::instancia()->selectAsientomodelofechacab_as_modeloasientomodelo(nombre);
 if ( query.isActive() )
    if (query.next())
       {
        addElementoTextoDom(doc,tag,"Nombre",filtracadxml(query.value(0).toString()));
        addElementoTextoDom(doc,tag,"Fecha",query.value(1).toString());
        addElementoTextoDom(doc,tag,"Aib",query.value(2).toBool() ? "true" : "false");
        addElementoTextoDom(doc,tag,"Autofactura_ue",query.value(3).toBool() ? "true" : "false");
        addElementoTextoDom(doc,tag,"Autofactura_no_ue",query.value(4).toBool() ? "true" : "false");
        addElementoTextoDom(doc,tag,"Eib",query.value(5).toBool() ? "true" : "false");
        addElementoTextoDom(doc,tag,"Eib_servicios",query.value(6).toBool() ? "true" : "false");
        addElementoTextoDom(doc,tag,"Diario",query.value(7).toString());
        addElementoTextoDom(doc,tag,"Importacion",query.value(8).toBool() ? "true" : "false");
        addElementoTextoDom(doc,tag,"Exportacion",query.value(9).toBool() ? "true" : "false");
        addElementoTextoDom(doc,tag,"Isp_interior",query.value(10).toBool() ? "true" : "false");
       }

 // variables
 QDomElement tag2 = doc.createElement("Variables");
 root.appendChild(tag2);

    query = basedatos::instancia()->selectVariabletipodescripcionvar_as_modeloasientomodelo(nombre);
   if ( query.isActive() )
       while (query.next())
          {
              QDomElement tag3 = doc.createElement("Variable");
              tag2.appendChild(tag3);
              // variable,tipo,descripcion,valor,orden,guardar
              addElementoTextoDom(doc,tag3,"NombreVar",filtracadxml(query.value(0).toString()));
              addElementoTextoDom(doc,tag3,"Tipo",filtracadxml(query.value(1).toString()));
              addElementoTextoDom(doc,tag3,"Descripcion",filtracadxml(query.value(2).toString()));
              addElementoTextoDom(doc,tag3,"Valor",filtracadxml(query.value(3).toString()));
              addElementoTextoDom(doc,tag3,"Orden",query.value(4).toString());
              addElementoTextoDom(doc,tag3,"Guardar",query.value(5).toBool() ? "true" : "false");
         }

 
 // detalle asiento
 QDomElement tagd = doc.createElement("Detalle");
 root.appendChild(tagd);
 query = basedatos::instancia()->select_det_as_modeloasientomodelo(nombre);
   if ( query.isActive() )
       while (query.next())
          {
              QDomElement tag3 = doc.createElement("Linea");
              tagd.appendChild(tag3);
              // cuenta,concepto,expresion,d_h,ci,baseiva,cuentafra,cuentabaseiva,
              // claveiva,documento,fecha_factura
              addElementoTextoDom(doc,tag3,"Cuenta",filtracadxml(query.value(0).toString()));
              addElementoTextoDom(doc,tag3,"Concepto",filtracadxml(query.value(1).toString()));
              addElementoTextoDom(doc,tag3,"Expresion",filtracadxml(query.value(2).toString()));
              addElementoTextoDom(doc,tag3,"DH",query.value(3).toString());
              addElementoTextoDom(doc,tag3,"CI",filtracadxml(query.value(4).toString()));
              addElementoTextoDom(doc,tag3,"BaseIva",filtracadxml(query.value(5).toString()));
              addElementoTextoDom(doc,tag3,"CuentaFra",filtracadxml(query.value(6).toString()));
              addElementoTextoDom(doc,tag3,"CuentaBaseIva",filtracadxml(query.value(7).toString()));
              addElementoTextoDom(doc,tag3,"ClaveIva",filtracadxml(query.value(8).toString()));
              addElementoTextoDom(doc,tag3,"Documento",filtracadxml(query.value(9).toString()));
              addElementoTextoDom(doc,tag3,"FechaFra",filtracadxml(query.value(10).toString()));
              addElementoTextoDom(doc,tag3,"Prorrata_e",filtracadxml(query.value(11).toString()));
              addElementoTextoDom(doc,tag3,"Rectificativa",
                                  query.value(12).toBool() ? "true" : "false");
              // addElementoTextoDom(doc,tag3,"Autofactura",query.value(13).toBool() ? "true" : "false");
              addElementoTextoDom(doc,tag3,"Fecha_op",filtracadxml(query.value(13).toString()));
              addElementoTextoDom(doc,tag3,"Clave_op",filtracadxml(query.value(14).toString()));
              addElementoTextoDom(doc,tag3,"Bicoste",filtracadxml(query.value(15).toString()));
              addElementoTextoDom(doc,tag3,"Rectificada",filtracadxml(query.value(16).toString()));
              addElementoTextoDom(doc,tag3,"Numfacts",filtracadxml(query.value(17).toString()));
              addElementoTextoDom(doc,tag3,"Facini",filtracadxml(query.value(18).toString()));
              addElementoTextoDom(doc,tag3,"Facfinal",filtracadxml(query.value(19).toString()));
              addElementoTextoDom(doc,tag3,"BienInversion",
                                  query.value(20).toBool() ? "true" : "false");
              // autofactura, afecto, agrario, nombre,  cif, "
              //"cta_retenido, arrendamiento, clave, base_ret, tipo_ret, "
              // "retencion, ing_a_cta, ing_a_cta_rep, nombre_ret, cif_ret, provincia "
              addElementoTextoDom(doc,tag3,"Afecto",filtracadxml(query.value(21).toString()));
              addElementoTextoDom(doc,tag3,"Agrario",
                                  query.value(22).toBool() ? "true" : "false");
              addElementoTextoDom(doc,tag3,"Nombre",filtracadxml(query.value(23).toString()));
              addElementoTextoDom(doc,tag3,"Cif",filtracadxml(query.value(24).toString()));
              addElementoTextoDom(doc,tag3,"Cta_retenido",filtracadxml(query.value(25).toString()));
              addElementoTextoDom(doc,tag3,"Arrendamiento",
                                  query.value(26).toBool() ? "true" : "false");
              addElementoTextoDom(doc,tag3,"Clave",filtracadxml(query.value(27).toString()));
              addElementoTextoDom(doc,tag3,"Base_ret",filtracadxml(query.value(28).toString()));
              addElementoTextoDom(doc,tag3,"Tipo_ret",filtracadxml(query.value(29).toString()));
              addElementoTextoDom(doc,tag3,"Retencion",filtracadxml(query.value(30).toString()));
              addElementoTextoDom(doc,tag3,"Ing_a_cta",filtracadxml(query.value(31).toString()));
              addElementoTextoDom(doc,tag3,"Ing_a_cta_rep",filtracadxml(query.value(32).toString()));
              addElementoTextoDom(doc,tag3,"Nombre_ret",filtracadxml(query.value(33).toString()));
              addElementoTextoDom(doc,tag3,"Cif_ret",filtracadxml(query.value(34).toString()));
              addElementoTextoDom(doc,tag3,"Provincia",filtracadxml(query.value(35).toString()));

         }

  QString xml = doc.toString();
  // -----------------------------------------------------------------------------------

  QFile fichero( adapta(nfich)  );

  if ( !fichero.open( QIODevice::WriteOnly ) ) return false;

  QTextStream stream( &fichero );
  stream.setCodec("UTF-8");

  stream << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
  stream << "<?xml-stylesheet type=\"text/xsl\" href=\"automat2html.xslt\"?>\n";

  // bool esmysql = ( basedatos::instancia()->cualControlador() == basedatos::MYSQL );

  // ------------------------------------------------------------------------------------
  stream << xml;
  fichero.close();

           // Vemos si hace falta copiar el fichero xslt
           QString fichxslt=dirtrabajobd();
           fichxslt.append(QDir::separator());
           fichxslt.append("automat2html.xslt");
           if (!QFile::exists(fichxslt))
              {
               QString pasa2;
               pasa2=traydoc();
               pasa2.append(QDir::separator());
               pasa2.append("automat2html.xslt");
               if (eswindows()) pasa2=QFile::encodeName(pasa2);
               QFile fichero(pasa2);
               if (eswindows()) fichero.copy(QFile::encodeName(fichxslt));
                  else fichero.copy(fichxslt);
              }


  return true;
}


int importarasmodelo(QString nombre)
{
  QDomDocument doc("AsientoAutomatico");
  QFile fichero(adapta(nombre));
  if (!fichero.exists()) return false;

  if ( !fichero.open( QIODevice::ReadOnly ) ) return false;

  if (!doc.setContent(&fichero)) {
     fichero.close();
     return false;
    }
 fichero.close();


 QString qnombre=etiquetadenodo(doc,"Nombre");
 // buscamos el nombre
 if ( basedatos::instancia()->asientomodeloencab_as_modelo(qnombre) == 2 ) {return 2; }
 QString asientomodelo,fecha;
 asientomodelo=qnombre;
 fecha=etiquetadenodo(doc,"Fecha");
 bool aib= etiquetadenodo(doc,"Aib") == "true" ? true : false;
 bool autofactura_ue= etiquetadenodo(doc,"Autofactura_ue") == "true" ? true : false;
 bool autofactura_no_ue = etiquetadenodo(doc,"Autofactura_no_ue") == "true" ? true : false;
 bool eib= etiquetadenodo(doc,"Eib") == "true" ? true : false;
 bool eib_servicios= etiquetadenodo(doc,"Eib_servicios") == "true" ? true : false;
 QString diario=etiquetadenodo(doc,"Diario");
 bool importacion=etiquetadenodo(doc,"Importacion") == "true" ? true : false;
 bool exportacion=etiquetadenodo(doc,"Exportacion") == "true" ? true : false;
 bool isp_op_interiores=etiquetadenodo(doc,"Isp_interior") == "true" ? true : false;

 //addElementoTextoDom(doc,tag,"Aib",query.value(2).toString());
 // addElementoTextoDom(doc,tag,"Autofactura_ue",query.value(3).toString());
 //addElementoTextoDom(doc,tag,"Autofactura_no_ue",query.value(4).toString());
 //addElementoTextoDom(doc,tag,"Eib",query.value(5).toString());
 //addElementoTextoDom(doc,tag,"Eib_servicios",query.value(6).toString());
 //addElementoTextoDom(doc,tag,"Diario",query.value(7).toString());

 // QString asientomodelo, QString fecha, QString tipo,
 // bool aib, bool autofactura_ue, bool autofactura_no_ue,
 // bool eib, bool eib_servicios, QString diario
 basedatos::instancia()->insertCab_as_modelo(asientomodelo,fecha,
                                             QObject::tr("GENERAL"), aib, autofactura_ue,
                                             autofactura_no_ue, eib, eib_servicios,
                                             isp_op_interiores,importacion, exportacion,
                                             diario);

 // pasamos a importar variables
 QDomNodeList listanodosnombreVar=doc.elementsByTagName ("NombreVar");
 QDomNodeList listanodostipo=doc.elementsByTagName ("Tipo");
 QDomNodeList listanodosdescripcion=doc.elementsByTagName ("Descripcion");
 QDomNodeList listanodosvalor=doc.elementsByTagName ("Valor");
 QDomNodeList listanodosorden=doc.elementsByTagName ("Orden");
 QDomNodeList listanodosguardar=doc.elementsByTagName ("Guardar");

 for (int veces=0; veces<listanodosnombreVar.count();veces++)
     {
       QString variable=textodenodo(listanodosnombreVar.at(veces));
       QString tipo=textodenodo(listanodostipo.at(veces));
       QString descripcion=textodenodo(listanodosdescripcion.at(veces));
       QString valor=textodenodo(listanodosvalor.at(veces));
       int orden=textodenodo(listanodosorden.at(veces)).toInt();
       bool guardar;
       guardar=textodenodo(listanodosguardar.at(veces)) == "true" ? true : false;
       basedatos::instancia()->insertVar_as_modelo(asientomodelo, variable,
                                 tipo, descripcion,
                                 valor, guardar,orden);
     }

 // importamos líneas de detalle
 QDomNodeList listanodoscuenta=doc.elementsByTagName ("Cuenta");
 QDomNodeList listanodosconcepto=doc.elementsByTagName ("Concepto");
 QDomNodeList listanodosexpresion=doc.elementsByTagName ("Expresion");
 QDomNodeList listanodosd_h=doc.elementsByTagName ("DH");
 QDomNodeList listanodosci=doc.elementsByTagName ("CI");
 QDomNodeList listanodosbaseiva=doc.elementsByTagName ("BaseIva");
 QDomNodeList listanodoscuentafra=doc.elementsByTagName ("CuentaFra");
 QDomNodeList listanodoscuentabaseiva=doc.elementsByTagName ("CuentaBaseIva");
 QDomNodeList listanodosclaveiva=doc.elementsByTagName ("ClaveIva");
 QDomNodeList listanodosdocumento=doc.elementsByTagName ("Documento");
 QDomNodeList listanodosfecha_factura=doc.elementsByTagName ("FechaFra");
 QDomNodeList listanodosprorrata_e=doc.elementsByTagName ("Prorrata_e");
 QDomNodeList listanodosrectificativa=doc.elementsByTagName ("Rectificativa");
 // QDomNodeList listanodosautofactura=doc.elementsByTagName ("Autofactura");
 QDomNodeList listanodosfecha_op=doc.elementsByTagName ("Fecha_op");
 QDomNodeList listanodosclave_op=doc.elementsByTagName ("Clave_op");
 QDomNodeList listanodosbicoste=doc.elementsByTagName ("Bicoste");
 QDomNodeList listanodosrectificada=doc.elementsByTagName ("Rectificada");
 QDomNodeList listanodosnumfacts=doc.elementsByTagName ("Numfacts");
 QDomNodeList listanodosfacini=doc.elementsByTagName ("Facini");
 QDomNodeList listanodosfacfinal=doc.elementsByTagName ("Facfinal");
 QDomNodeList listanodosbinversion=doc.elementsByTagName ("BienInversion");
 QDomNodeList listanodosafecto=doc.elementsByTagName ("Afecto");
 QDomNodeList listanodosagrario=doc.elementsByTagName ("Agrario");
 QDomNodeList listanodosnombre=doc.elementsByTagName ("Nombre");
 QDomNodeList listanodoscif=doc.elementsByTagName ("Cif");
 QDomNodeList listanodoscta_retenido=doc.elementsByTagName ("Cta_retenido");
 QDomNodeList listanodosarrendamiento=doc.elementsByTagName ("Arrendamiento");
 QDomNodeList listanodosclave=doc.elementsByTagName ("Clave");
 QDomNodeList listanodosbase_ret=doc.elementsByTagName ("Base_ret");
 QDomNodeList listanodostipo_ret=doc.elementsByTagName ("Tipo_ret");
 QDomNodeList listanodosretencion=doc.elementsByTagName ("Retencion");
 QDomNodeList listanodosing_a_cta=doc.elementsByTagName ("Ing_a_cta");
 QDomNodeList listanodosing_a_cta_rep=doc.elementsByTagName ("Ing_a_cta_rep");
 QDomNodeList listanodosnombre_ret=doc.elementsByTagName ("Nombre_ret");
 QDomNodeList listanodoscif_ret=doc.elementsByTagName ("Cif_ret");
 QDomNodeList listanodosprovincia=doc.elementsByTagName ("Provincia");


 for (int veces=0; veces<listanodoscuenta.count();veces++)
     {

      QString cuenta,concepto,expresion,d_h,ci,baseiva,cuentafra,cuentabaseiva,claveiva,documento;
      QString fecha_factura, prorrata_e;
      bool rectificativa, binversion;
      QString fecha_op, clave_op, bicoste;
      QString rectificada, numfacts, facini, facfinal;

      // afecto, agrario, nombre,cif, cta_retenido, arrendamiento,
      //             clave, base_ret, tipo_ret, retencion, ing_a_cta, ing_a_cta_rep, nombre_ret,
      //             cif_ret, provincia

      QString afecto;
      bool agrario;
      QString nombre, cif, cta_retenido;
      bool arrendamiento;
      QString clave, base_ret, tipo_ret, retencion, ing_a_cta;
      QString ing_a_cta_rep, nombre_ret, cif_ret, provincia;

      cuenta=textodenodo(listanodoscuenta.at(veces));
      concepto=textodenodo(listanodosconcepto.at(veces)).replace("&quot;","\"");
      expresion=textodenodo(listanodosexpresion.at(veces));
      d_h=textodenodo(listanodosd_h.at(veces));
      ci=textodenodo(listanodosci.at(veces));
      baseiva=textodenodo(listanodosbaseiva.at(veces));
      cuentafra=textodenodo(listanodoscuentafra.at(veces));
      cuentabaseiva=textodenodo(listanodoscuentabaseiva.at(veces));
      claveiva=textodenodo(listanodosclaveiva.at(veces));
      documento=textodenodo(listanodosdocumento.at(veces));
      fecha_factura=textodenodo(listanodosfecha_factura.at(veces));
      prorrata_e=textodenodo(listanodosprorrata_e.at(veces));
      rectificativa=textodenodo(listanodosrectificativa.at(veces))=="true" ? true : false;
      // autofactura=textodenodo(listanodosautofactura.at(veces))=="true" ? true : false;
      fecha_op=textodenodo(listanodosfecha_op.at(veces));
      clave_op=textodenodo(listanodosclave_op.at(veces));
      bicoste=textodenodo(listanodosbicoste.at(veces));


      rectificada=textodenodo(listanodosrectificada.at(veces));
      numfacts=textodenodo(listanodosnumfacts.at(veces));
      facini=textodenodo(listanodosfacini.at(veces));
      facfinal=textodenodo(listanodosfacfinal.at(veces));
      binversion=textodenodo(listanodosbinversion.at(veces))=="true" ? true : false;


      afecto=textodenodo(listanodosafecto.at(veces));
      agrario=textodenodo(listanodosagrario.at(veces))=="true" ? true : false;
      nombre=textodenodo(listanodosnombre.at(veces));
      cif=textodenodo(listanodoscif.at(veces));
      cta_retenido=textodenodo(listanodoscta_retenido.at(veces));
      arrendamiento=textodenodo(listanodosarrendamiento.at(veces))=="true" ? true : false;
      clave=textodenodo(listanodosclave.at(veces));
      base_ret=textodenodo(listanodosbase_ret.at(veces));
      tipo_ret=textodenodo(listanodostipo_ret.at(veces));
      retencion=textodenodo(listanodosretencion.at(veces));
      ing_a_cta=textodenodo(listanodosing_a_cta.at(veces));
      ing_a_cta_rep=textodenodo(listanodosing_a_cta_rep.at(veces));
      nombre_ret=textodenodo(listanodosnombre_ret.at(veces));
      cif_ret=textodenodo(listanodoscif_ret.at(veces));
      provincia=textodenodo(listanodosprovincia.at(veces));


      basedatos::instancia()->insertDet_as_modelo (asientomodelo , cuenta ,
            concepto , expresion , d_h , ci , baseiva , cuentabaseiva , cuentafra ,
            claveiva , documento, fecha_factura, prorrata_e, rectificativa,
            fecha_op,clave_op, bicoste, rectificada, numfacts, facini, facfinal,
            binversion, afecto, agrario, nombre,cif, cta_retenido, arrendamiento,
            clave, base_ret, tipo_ret, retencion, ing_a_cta, ing_a_cta_rep, nombre_ret,
            cif_ret, provincia,
            veces);

     }
 return 1;
}


int existecodigodiario(QString cadena,QString *qdescrip)
{
    return basedatos::instancia()->existecodigodiario(cadena,qdescrip);
}

void guardacambiosdiario(QString codigo,QString descripcion)
{
    basedatos::instancia()->updateDiariosdescripcioncodigo(codigo,descripcion);
}

void insertaendiarios(QString codigo,QString descripcion)
{
    basedatos::instancia()->insertDiarios(codigo,descripcion);
}

int eliminadiario(QString qcodigo)
{
    return basedatos::instancia()->eliminadiario(qcodigo);
}

QString noejercicio()
{
 return "- -";
}

bool existectapres(QString cuenta,QString ejercicio)
{
    return basedatos::instancia()->existectapres(cuenta,ejercicio);
}

double prevision(QString codigo,QString ejercicio)
{
    QSqlQuery query = basedatos::instancia()->selectPresupuestopresupuestoejerciciocuenta(codigo,ejercicio);

    if ( (query.isActive()) && (query.first()) )
    {
        return query.value(0).toDouble();
    }
    else
    {
       query = basedatos::instancia()->selectSumpresupuestopresupuestoejerciciocuenta(codigo,ejercicio);
       if ( (query.isActive()) &&(query.first()) ) return query.value(0).toDouble();
    }
    return 0;
}


double previsionsegunsaldo(QString codigo,QString ejercicio,bool saldopos)
{
    double total=0;
    QSqlQuery query = basedatos::instancia()->selectPresupuestopresupuestoejerciciocuenta(codigo,ejercicio);
    if ( (query.isActive()) && (query.first()) )
    {
        double valor=query.value(0).toDouble();
        if (saldopos) 
        {
            if (valor > 0) return valor; else return 0;
        }
        else 
        {
            if (valor< 0) return valor ; else return 0;
        }
    }
    else
    {
        query = basedatos::instancia()->selectPresupuestopresupuestoEjercicioPosCuenta(codigo,ejercicio);
        if (query.isActive()) {
            while (query.next()) 
            {
                if (saldopos && query.value(0).toDouble()>0) total+=query.value(0).toDouble();
                if (saldopos && query.value(0).toDouble()<0) total+=query.value(0).toDouble();
            }
        }
    }
    return total;
}


double previsionsegunsaldo_aux(QString codigo,QString ejercicio,bool saldopos)
{
    QSqlQuery query = basedatos::instancia()->selectPresupuestopresupuestoEjercicioCuenta(codigo,ejercicio);
    if ( (query.isActive()) && (query.first()) )
    {
        double valor=query.value(0).toDouble();
        if (saldopos) 
        {
            if (valor > 0) return valor; else return 0;
        }
        else 
        {
            if (valor< 0) return valor ; else return 0;
        }
    }
    return 0;
}


QString comandodescrip()
{
    return QObject::tr("DESCRIP");
}

bool cod_longitud_variable()
{
    return anchocuentas()==0;
}

bool esauxiliar(QString codigo)
{
    if (cod_longitud_variable())
    {
        QSqlQuery query = basedatos::instancia()->selectCodigoauxiliarplancontablecodigo(codigo);
        if ( (query.isActive()) && (query.first()) )
        {
            return query.value(1).toBool();
        }
    }
    else return codigo.length()==anchocuentas();
    return false;
}

bool es_long_var_agregadora(QString codigo)
{
    QSqlQuery query = basedatos::instancia()->selectCodigoauxiliarplancontablecodigo(codigo);
    if ( (query.isActive()) && (query.first()) )
    {
        return query.value(1).toBool();
    }
    return 0; 
}


bool prorrata_especial()
{
    return basedatos::instancia()->prorrata_especial();
}

double prorrata_iva()
{
    return basedatos::instancia()->prorrata_iva();
}


bool cuentaenplanamort(QString codigo)
{
    return basedatos::instancia()->cuentaenplanamort(codigo);
}

bool compronivelsup()
{
 return COMPRONIVELSUP ;
}


bool existecodigodif_conc(QString cadena)
{
    return basedatos::instancia()->existecodigodif_conc(cadena);
}

void eliminaconceptodif(QString qcodigo)
{
    basedatos::instancia()->deleteDif_conciliacioncodigo(qcodigo);
}


bool conceptodif_usado(QString cadena)
{
    QSqlQuery query = basedatos::instancia()->selectDif_conciliaciondiariodif_conciliacion(cadena);
    if ( (query.isActive()) && (query.first()) )
    {
        return true;
    }

    query = basedatos::instancia()->selectDif_conciliacioncuenta_ext_concidif_conciliacion(cadena);
    if ( (query.isActive()) && (query.first()) )
    {
        return true;
    }

    query = basedatos::instancia()->selectDif_conciliacionajustes_concidif_conciliacion(cadena);
    if ( (query.isActive()) && (query.first()) )
    {
        return true;
    }

    return false;
}


void guardacambiosdif_conc(QString codigo,QString descripcion)
{
    basedatos::instancia()->updateDif_conciliaciondescripcioncodigo(codigo,descripcion);
}


void insertaendif_conc(QString codigo,QString descripcion)
{
    basedatos::instancia()->insertDif_conciliacion(codigo,descripcion);
}


QString descripciondiferencia(QString qcodigo)
{
    return basedatos::instancia()->selectDescripciondif_conciliacioncodigo(qcodigo);
}


bool cuentaenpresupuesto(QString cadena)
{
    return basedatos::instancia()->cuentaenpresupuesto(cadena);
}


bool cuentaen_ajustes_conci(QString cadena)
{
    return basedatos::instancia()->cuentaen_ajustes_conci(cadena);
}


bool cuentaencuenta_ext_conci(QString cadena)
{
    return basedatos::instancia()->cuentaencuenta_ext_conci(cadena);
}

bool es_sqlite()
{
    return ( basedatos::instancia()->cualControlador() == basedatos::SQLITE );
}

QString usuario_sistema()
{
 QString usuario;
 if (WINDOWS) usuario=usuario.fromLocal8Bit(getenv("USER"));
      else usuario=getenv("USER");
 return usuario;
}

QString nombrebd()
{
    return basedatos::instancia()->nombre();
}

QString fichconex()
{
     QString dirconfig=trayconfig();

     QString nombredir="/conexiones.cfg";
     nombredir[0]=QDir::separator();
     return dirconfig+nombredir;
}


QString tipo_exento()
{
  return QObject::tr("EXENTO");
}

QString tipo_exento_nodeduc()
{
  return QObject::tr("EXENT_NO_DED");
}

QString tipo_nosujeto()
{
  return QObject::tr("NO_SUJETO");
}


QString cabeceralatex()
{
 QString cadena;
    cadena="\\documentclass[11pt,a4paper]{article}\n"
           "\\usepackage{ucs}\n"
           "\\usepackage[utf8x]{inputenc}\n"
           "\\usepackage[spanish]{babel}\n"
           "\\usepackage{lscape}\n"
           "\\usepackage{longtable}\n"
           "\\usepackage{anysize}\n"
           "\\usepackage{textcomp}\n"
           "\\begin{document}\n"
           "% CUERPO\n";
 return cadena;
}

QString cabeceralatex_graficos()
{
 QString cadena;
    cadena="\\documentclass[11pt,a4paper]{article}\n"
           "\\usepackage{ucs}\n"
           "\\usepackage[utf8x]{inputenc}\n"
           "\\usepackage[spanish]{babel}\n"
           "\\usepackage{lscape}\n"
           "\\usepackage{longtable}\n"
           "\\usepackage{anysize}\n"
           "\\usepackage{textcomp}\n"
           "\\usepackage{graphicx}\n"
           "\\begin{document}\n"
           "% CUERPO\n";
 return cadena;
}


QString margen_latex()
 {
   // margen para la memoria
   return "\\marginsize{3cm}{2cm}{2cm}{2cm}\n";
 }

QString margen_extra_latex()
 {
   // margen para listados contables
   return "\\marginsize{1.5cm}{1cm}{2cm}{2cm}\n";
 }


void ejecuta_regularizacion(QString cuenta_reg, QString cuentas, QString descripcion,
                            QDate fechainicial,QDate fechafinal,QDate fechaasiento, QString diario)
{

    if (!fechacorrespondeaejercicio(fechaasiento))
      {
	QMessageBox::warning( 0, QObject::tr("Tabla de apuntes"),QObject::tr("ERROR: La fecha del asiento no corresponde a ningún ejercicio definido"));
	return;
      }

   if (ejerciciocerrado(ejerciciodelafecha(fechaasiento)) ||
        ejerciciocerrando(ejerciciodelafecha(fechaasiento)))
    {
         QMessageBox::warning( 0, QObject::tr("Regularización"),
                                      QObject::tr("Error, el ejercicio seleccionado está cerrado o\n"
				  "se está cerrando."));
         return;
    }

 if (basedatos::instancia()->fecha_periodo_bloqueado((fechaasiento)))
    {
         QMessageBox::warning( 0, QObject::tr("Regularización"),
                               QObject::tr("Error, la fecha suministrada pertenece a periodo bloqueado"));
         return;
    }

  if (basedatos::instancia()->existe_bloqueo_cta(cuenta_reg,fechaasiento))
     {
       QMessageBox::warning( 0, QObject::tr("Regularización"),QObject::tr("ERROR EN FASE: La cuenta %1 "
                               "está bloqueada").arg(cuenta_reg));
                             return;
     }

  if (sobreescribeproxasiento(ejerciciodelafecha(fechaasiento)))
       {
              QMessageBox::warning( 0, QObject::tr("Regularización"),
				   QObject::tr("Error, no se pueden sobreescribir asientos\n"
				      "revise número de próximo asiento en configuración."));
             return;
      }

   QString cadinicio=fechainicial.toString("yyyy-MM-dd");
   QString cadfin=fechafinal.toString("yyyy-MM-dd");
   QString cadfechaasiento=fechaasiento.toString("yyyy-MM-dd");
   QString filtro;
   QString ejercicio=ejerciciodelafecha(fechaasiento);
   int partes=cuentas.count(',');
   if (cuentas.length()>0)
      {
       for (int veces=0;veces<=partes;veces++)
           {
             if (veces==0) filtro=" and (";
             if (basedatos::instancia()->cualControlador() == basedatos::SQLITE) {
                 filtro += "codigo like '"+ cuentas.section(',',veces,veces).replace("'","''") +"%'";
                }
                else {
                       filtro+="position('";
                       filtro+=cuentas.section(',',veces,veces).replace("'","''");
                       filtro+="' in codigo)=1";
                     }
             if (veces!=partes)          // filtrogastos+=")";
                  filtro+=" or ";
            }
         filtro+=")";
        }
 // -------------------------------------------------------------------------------------------------   

   // empezamos una nueva transacción y averiguamos número de asiento
   // QSqlDatabase contabilidad=QSqlDatabase::database();
   // contabilidad.transaction();

   // basedatos::instancia()->bloquea_para_regularizacion();

    qlonglong vnum = basedatos::instancia()->proximoasiento(ejercicio);

    QString cadnum; cadnum.setNum(vnum+1);
    QString cadnumasiento; cadnumasiento.setNum(vnum);

    basedatos::instancia()->update_ejercicio_prox_asiento(ejercicio, cadnum);
   QString cadanchocuentas; cadanchocuentas.setNum(anchocuentas());
   QSqlQuery query = basedatos::instancia()->selectCodigoplancontablelengthauxiliarfiltro(cadanchocuentas, filtro);
   QString codigo;
   int veces=0;
   double saldo=0;
   QString cadsaldo;

    if ( query.isActive() ) {
          while ( query.next() )
               {
                 veces++;

                 codigo=query.value(0).toString();
                 // vamos calculando el saldo de las cuentas para el intervalo de fechas
                 QSqlQuery query2 = basedatos::instancia()->selectSaldocuentaendiariofechas(codigo,
                           cadinicio, cadfin);
                 if (query2.isActive())
                 if (query2.next())
                     {
                        if (query2.value(0).toDouble()>0.001 || query2.value(0).toDouble()<-0.001)
                        {
                         // vemos si la cuenta está bloqueada
                         if (basedatos::instancia()->existe_bloqueo_cta(codigo,fechaasiento))
                            {
                              QMessageBox::warning( 0, QObject::tr("Regularización"),QObject::tr("ERROR EN FASE: La cuenta %1 "
                                               "está bloqueada").arg(codigo));
                              return;
                            }
                        }
                    }
              }
      }

   query = basedatos::instancia()->selectCodigoplancontablelengthauxiliarfiltro(cadanchocuentas, filtro);
   veces=0;
   saldo=0;
   cadsaldo.clear();


    if ( query.isActive() ) {
          while ( query.next() )
               {
	         veces++;

	         codigo=query.value(0).toString();
	         // vamos calculando el saldo de las cuentas para el intervalo de fechas
	         QSqlQuery query2 = basedatos::instancia()->selectSaldocuentaendiariofechas(codigo,
                           cadinicio, cadfin);
	         if (query2.isActive())
	         if (query2.next())
		     {
		        if (query2.value(0).toDouble()>0.001 || query2.value(0).toDouble()<-0.001)
		        {
			 // insertamos nuevo apunte
			 saldo+=query2.value(0).toDouble();
                         qlonglong pase = basedatos::instancia()->selectProx_paseconfiguracion();
                         QString cadnumpase;
                         cadnumpase.setNum(pase);

                         QString qdebe="0";
                         QString qhaber="0";
			 if (query2.value(0).toDouble()<0) 
			        { 
			            qdebe = query2.value(0).toString();
			            qdebe.remove('-');
			        }
			 if (query2.value(0).toDouble()>0) qhaber = query2.value(0).toString();
                         basedatos::instancia()->insertDiarioRegulParc(cadnumasiento,
                                                                       cadnumpase,
                          cadfechaasiento, codigo, qdebe, qhaber,descripcion, ejercicio, diario);
			        // queda actu saldo y proxpase
			        // queda actu saldo y proxpase

			 if (query2.value(0).toDouble()>0)
                            {
                             basedatos::instancia()->updateSaldossubcuentasaldocargocodigo(
                                codigo, query2.value(0).toString(), false);
                            }
                           else 
                                {
				  QString lacad=query2.value(0).toString();
				  lacad.remove('-');
                                  basedatos::instancia()->updateSaldossubcuentasaldocargocodigo(codigo,
                                     lacad, true);
			        }

                        // actu proxpase
                        pase++;
                        cadnumpase.setNum(pase);
                        basedatos::instancia()->updateConfiguracionprox_pase(cadnumpase);
			 // --------------------------------------------------
		        } // if value
		      } // if query2 next
	      } // while query next
          } // if query isactive
    // ---------------------------------
    if (saldo>0.001 || saldo<-0.001)
      {
        // contabilizamos contrapartida
         QString cadnumpase;
         qlonglong pase = basedatos::instancia()->selectProx_paseconfiguracion();
         cadnumpase.setNum(pase);

         QString qdebe="0";
         QString qhaber="0";
         cadsaldo.setNum(saldo,'f',2);
         if (saldo>0) qdebe = cadsaldo;
         else qdebe = "0";
         if (saldo<0) 
           { 
            cadsaldo.remove('-');
            qhaber = cadsaldo; 
           }
	   else qhaber = "0";

         basedatos::instancia()->insertDiarioRegulParc(cadnumasiento, cadnumpase,
                    cadfechaasiento, cuenta_reg, qdebe, qhaber,descripcion, ejercicio,diario);

         // queda actu saldo y proxpase
        if (saldo>0) 
	    {
            basedatos::instancia()->updateSaldossubcuentasaldocargocodigo(codigo, cadsaldo, true);
        }
        else 
	    {
            basedatos::instancia()->updateSaldossubcuentasaldocargocodigo(codigo, cadsaldo, false);
	    }

          // actu proxpase
          pase++;
          cadnumpase.setNum(pase);
          basedatos::instancia()->updateConfiguracionprox_pase(cadnumpase);

      //----------------------------------------------------------------------------------------------
      }
   // basedatos::instancia()->desbloquea_y_commit();

}


QString xmlAbreTagLin(int pos,QString cadena)
{
  QString salida="<"+cadena;
  for (int veces=0;veces<pos*2;veces++) salida.prepend(' ');
  salida.append(">\n");
  return salida;
 }
 
QString xmlCierraTagLin(int pos,QString cadena)
 {
  QString salida="</"+cadena;
  for (int veces=0;veces<pos*2;veces++) salida.prepend(' ');
  salida.append(">\n");
  return salida;
 }
 
 
QString xmlAbreTag(int pos,QString cadena)
 {
  QString salida="<"+cadena;
  for (int veces=0;veces<pos*2;veces++) salida.prepend(' ');
  salida.append(">");
  return salida;
 }
 
 
QString xmlCierraTag(QString cadena)
 {
  QString salida="</"+cadena;
  salida.append(">\n");
  return salida;
 }

void addElementoTextoDom(QDomDocument doc,QDomElement padre,QString tagnombre,QString tagtexto)
{
  QDomElement taghijo = doc.createElement(tagnombre);
  padre.appendChild(taghijo);
  QDomText texthijo = doc.createTextNode(tagtexto);
  taghijo.appendChild(texthijo);
}


QString filtracadxml(QString cadena)
{
// Y comercial 	&amp; 	& 	&#38;#38;
// Abrir corchete angular 	&lt; 	< 	&#38;#60;
// Cerrar corchete angular 	&gt; 	> 	&#62;
// Comilla recta 	&quot; 	" 	&#39;
// Apóstrofo 	&apos; 	' 	&#34;
// cadena.replace("&","&amp;");
cadena.replace("<","&lt;");
cadena.replace(">","&gt;");
cadena.replace("'","&#39;"); // cadena.replace("'","&apos;");
cadena.replace('"',"&quot;");
return cadena;
}

QString cadena_descrip_ci (QString codigo)
{
     QString caddescripci;
     QString cadena,descripcion;
     QString qnivel=0;

     if (codigo.startsWith("???"))
                   {
                    caddescripci=QObject::tr("CUALQUIERA");
                   }
               else
                   {
                     bool encontrada=buscaci(codigo.left(3),&descripcion,&qnivel);
                     if (encontrada && qnivel.toInt()==1)
                         caddescripci=descripcion;
                   }
     if (codigo.length()>3) caddescripci += " - ";
     if (codigo.length()>3 && codigo.mid(3,3)==QString("???")) caddescripci += QObject::tr("CUALQUIERA");
               else
                    {
                     bool encontrada=buscaci(codigo.mid(3,3),&descripcion,&qnivel);
                     int elnivel=qnivel.toInt();
                     if (encontrada && elnivel==2)
                     caddescripci += descripcion;
                    }

     if (codigo.length()>6) caddescripci += " - ";
     if (codigo.length()==7 && codigo[6]=='*')  caddescripci += QObject::tr("CUALQUIERA");
        else
            {
              bool encontrada=buscaci(codigo.right(codigo.length()-6),&descripcion,&qnivel);
              if (encontrada && qnivel.toInt()==3)
                 caddescripci+=descripcion;
            }
    return caddescripci;
}

QString tipoivaexento()
{
    return QObject::tr("EXENTO");
}


bool hay_prorratageneral()
{
    if (prorrata_especial()) return false;
    double prorrata=prorrata_iva();
    if (prorrata>0.001 && prorrata< 9999) return true;
    return false;
}

void borrarasientofunc(QString elasiento, QString ejercicio)
{
    // QDate fechapase=fechadeasiento(elasiento,ejercicio);
    if (amortenintasientos(elasiento,elasiento,ejercicio))
       {
         QMessageBox::warning( 0, QObject::tr("BORRA ASIENTO"),
         QObject::tr("ERROR: El asiento seleccionado forma parte de la tabla de amortizaciones"));
         return;
       }

      // QString ejercicio=ejerciciodelafecha(fechapase);
      if (ejerciciocerrado(ejercicio) || ejerciciocerrando(ejercicio))
       {
         QMessageBox::warning( 0, QObject::tr("BORRA ASIENTO"),
                               QObject::tr("ERROR: El pase seleccionado corresponde a un ejercicio cerrado"));
         return;
       }
      if (basedatos::instancia()->asiento_en_periodo_bloqueado(elasiento,ejercicio))
       {
         QMessageBox::warning( 0, QObject::tr("EDICIÓN DE ASIENTOS"),
                               QObject::tr("ERROR: El asiento seleccionado corresponde a un periodo bloqueado"));
         return;
       }

      if (basedatos::instancia()->asiento_con_cuentas_bloqueadas(elasiento,ejercicio))
       {
         QMessageBox::warning( 0, QObject::tr("EDICIÓN DE ASIENTOS"),
                               QObject::tr("ERROR: El asiento seleccionado posee cuentas bloqueadas"));
         return;
       }

      //comprobamos enlace externo del asiento
      if (basedatos::instancia()->asiento_con_enlace(elasiento,ejercicio))
         {
          QMessageBox::warning( 0, QObject::tr("EDICIÓN DE ASIENTOS"),
                                QObject::tr("El asiento a editar está enlazado con una aplicación externa,\n"
                                            "la edición y borrado no se puede realizar desde KEME "
                                            ));
          return;
         }

  /*QString relas;
  if (numeracionrelativa())
      relas=numrelativa(elasiento);
    else relas=elasiento;*/

  if (!QMessageBox::question(
            0,
            QObject::tr("¿ Borrar asiento ?"),
            QObject::tr("¿ Desea borrar el asiento %1 ?").arg(elasiento),
            QObject::tr("&Sí"), QObject::tr("&No"),
            QString::null, 1, 1 ) ==0 )
                          return;
   borraasientos(elasiento,elasiento,ejercicio);
}

QString adaptacad_aeat(QString cadena)
{
  QString destino=cadena;
  destino.replace(QObject::tr("á"),"a");
  destino.replace(QObject::tr("é"),"e");
  destino.replace(QObject::tr("í"),"i");
  destino.replace(QObject::tr("ó"),"o");
  destino.replace(QObject::tr("ú"),"u");
  destino.replace(QObject::tr("Á"),"A");
  destino.replace(QObject::tr("É"),"E");
  destino.replace(QObject::tr("Í"),"I");
  destino.replace(QObject::tr("Ó"),"O");
  destino.replace(QObject::tr("Ú"),"U");
  destino.replace(QObject::tr("ñ"),QObject::tr("Ñ"));
  destino.replace(QObject::tr("ç"),QObject::tr("Ç"));
  destino.replace(","," ");
  destino=destino.toUpper();
  for (int veces=0; veces< destino.length(); veces++)
      if (!QObject::tr("ABCDEFGHIJKLMNÑOPQRSTUVWXYZÇ1234567890 ").contains(destino[veces]))
          destino.remove(destino[veces]);
  return destino;
}


QString completacadnum(QString cadena, int espacio)
{
   if (cadena.length()>espacio) cadena=cadena.left(espacio);
   int longitud=cadena.length();
   for (int veces=0 ; veces<espacio-longitud; veces++) cadena="0"+cadena;
   return cadena;
}

QString completacadcad(QString cadena, int espacio)
{
   if (cadena.length()>espacio) cadena=cadena.left(espacio);
   int longitud=cadena.length();
   for (int veces=0 ; veces<espacio-longitud; veces++) cadena=cadena+" ";
   return cadena;
}


QString rutadocs(void)
 {
  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;
  if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
  QFile ficheroini(nombrefichcompleto);
  QString ruta=0;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("RUTADOCS")==0) ruta=contenido;
              }
            }
        ficheroini.close();
       }
  return ruta;
}



QString rutacargadocs(void)
 {
  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;
  if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
  QFile ficheroini(nombrefichcompleto);
  QString ruta=0;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("RUTACARGADOCS")==0) ruta=contenido;
              }
            }
        ficheroini.close();
       }
  return ruta;
}

bool pref_forzar_ver_pdf()
{
    QString nombrefichcompleto=trayconfig();
    QString nombre="/keme.cfg";
    nombre[0]=QDir::separator();
    nombrefichcompleto+=nombre;
    if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
    QFile ficheroini(nombrefichcompleto);
    QString cad_forzar="";

     if ( ficheroini.open( QIODevice::ReadOnly ) ) {
          QTextStream stream( &ficheroini );
          stream.setCodec("UTF-8");
          QString linea,variable,contenido;
          while ( !stream.atEnd() ) {
              linea = stream.readLine(); // linea de texto excluyendo '\n'
              // analizamos la linea de texto
              if (linea.contains('=')) {
                 variable=linea.section('=',0,0);
                 contenido=linea.section('=',1,1);
                 if (variable.compare("FORZAR_VER_PDF")==0) cad_forzar=contenido;
                }
              }
          ficheroini.close();
         }
    bool forzar=false;
    if (cad_forzar=="1") forzar=true;
    return forzar;
}


QString rutacopiapdf(void)
 {
  if (basedatos::instancia()->gestiondeusuarios() && !privilegios[activa_visor_local])
      return QString();

  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;
  if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
  QFile ficheroini(nombrefichcompleto);
  QString ruta=0;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("RUTACOPIAPDF")==0) ruta=contenido;
              }
            }
        ficheroini.close();
       }
  return ruta;
}



QString qscript_copias(void)
 {

  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;
  if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
  QFile ficheroini(nombrefichcompleto);
  QString cadvalor;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("SCRIPT_COPIAS")==0) cadvalor=contenido;
              }
            }
        ficheroini.close();
       }
  return cadvalor;
}



QString rutacopiaseg(void)
 {
  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;
  if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
  QFile ficheroini(nombrefichcompleto);
  QString ruta=0;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("RUTASEG")==0) ruta=contenido;
              }
            }
        ficheroini.close();
       }
  return ruta;
}


QString rutalibros(void)
 {
  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;
  if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
  QFile ficheroini(nombrefichcompleto);
  QString ruta=0;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("RUTALIBROS")==0) ruta=contenido;
              }
            }
        ficheroini.close();
       }
  return ruta;
}



void visualizadores(QString *ext1, QString *vis1, QString *ext2, QString *vis2,
                    QString *ext3,QString *vis3, QString *ext4, QString *vis4)
{
  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;
  if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
  QFile ficheroini(nombrefichcompleto);
  QString ruta=0;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("EXT1")==0) *ext1=contenido;
               if (variable.compare("VIS1")==0) *vis1=contenido;
               if (variable.compare("EXT2")==0) *ext2=contenido;
               if (variable.compare("VIS2")==0) *vis2=contenido;
               if (variable.compare("EXT3")==0) *ext3=contenido;
               if (variable.compare("VIS3")==0) *vis3=contenido;
               if (variable.compare("EXT4")==0) *ext4=contenido;
               if (variable.compare("VIS4")==0) *vis4=contenido;
              }
            }
        ficheroini.close();
       }

}

QString aplicacionabrirfich(QString extension)
{
  QString ext1, vis1, ext2, vis2, ext3, vis3, ext4, vis4;

  visualizadores(&ext1, &vis1, &ext2, &vis2,
                 &ext3, &vis3, &ext4, &vis4);
  QStringList list;
  list = ext1.split(",");

  if (list.contains(extension)) return vis1;

  list = ext2.split(",");
  if (list.contains(extension)) return vis2;

  list = ext3.split(",");
  if (list.contains(extension)) return vis3;

  list = ext4.split(",");
  if (list.contains(extension)) return vis4;

  return QString();
}


bool ejecuta(QString aplicacion, QString qfichero)
{

    QObject *parent=NULL;

     QStringList arguments;
     arguments << adapta(qfichero);

     QProcess *myProcess = new QProcess(parent);

    if (! myProcess->startDetached(aplicacion,arguments))
       {
         delete myProcess;
         return false;
       }

    delete myProcess;


  return true;
}

QString extensionfich(QString fichero)
{
  int pospunto=fichero.lastIndexOf('.');
  if (!pospunto>0) return QString();
  return fichero.right(fichero.length()-pospunto-1);
}

QString expanderutadocfich(QString fichdoc)
{
  if (fichdoc.isEmpty()) return QString();
   QString caddestino=rutadocs();
   caddestino+=QDir::separator();
   if (nombrebd().length()>0)
          {
           if (!es_sqlite()) caddestino.append(nombrebd());
              else
                   {
                     // el nombre de la base de datos guarda toda la ruta
                     QString nbd;
                     nbd=nombrebd().right(nombrebd().length()-nombrebd().lastIndexOf(QDir::separator())-1);
                     caddestino.append(nbd);
                   }
          }
   caddestino+=QDir::separator();
   // averiguar número del documento
   int numdir=fichdoc.contains('-') ? (fichdoc.section('-',0,0).toInt()/100+1)*100 :
             (fichdoc.section('.',0,0).toInt()/100+1)*100 ;
   QString cadnumdir;
   cadnumdir.setNum(numdir);
   cadnumdir=cadnumdir.trimmed();
   caddestino+=cadnumdir;
   caddestino+=QDir::separator();
   caddestino+=fichdoc;
   return caddestino;
}


QString dirdocsdefecto()
{
   QString directorio;
   if (WINDOWS) directorio=directorio.fromLocal8Bit(getenv("USERPROFILE"));
     else
       directorio=getenv("HOME");
   QString nombredir="/keme";
   nombredir[0]=QDir::separator();
   directorio+=nombredir;
   directorio.append(QDir::separator());
   directorio+="docs";
   return directorio;
}


bool es_cuenta_para_analitica(QString codigo)
{
  bool procesar_analitica=false;
  if (conanalitica() && (escuentadegasto(codigo) || escuentadeingreso(codigo)))
     procesar_analitica=true;

  if (conanalitica_parc() && codigo_en_analitica(codigo))
     procesar_analitica=true;
  return procesar_analitica;
}


QString editor_latex()
{

  QString nombrefichcompleto=trayconfig();
  QString nombre="/keme.cfg";
  nombre[0]=QDir::separator();
  nombrefichcompleto+=nombre;

  if (eswindows()) nombrefichcompleto=QFile::encodeName(nombrefichcompleto);
  QFile ficheroini(nombrefichcompleto);
  QString qeditor;

   if ( ficheroini.open( QIODevice::ReadOnly ) ) {
        QTextStream stream( &ficheroini );
        stream.setCodec("UTF-8");
        QString linea,variable,contenido;
        while ( !stream.atEnd() ) {
            linea = stream.readLine(); // linea de texto excluyendo '\n'
            // analizamos la linea de texto
            if (linea.contains('=')) {
               variable=linea.section('=',0,0);
               contenido=linea.section('=',1,1);
               if (variable.compare("EDITORLATEX")==0) qeditor=contenido;
              }
            }
        ficheroini.close();
       }
  return qeditor;

}


int editalatex(QString qfichero)
{
    QString pasafich=dirtrabajo();
    pasafich.append(QDir::separator());
    pasafich.append(qfichero);
    pasafich.append(".tex");
    if (filtrartexaxtivado()) filtratex(pasafich);

    QString fichero=qfichero;

    QObject *parent=NULL;
    QProcess *myProcess = new QProcess(parent);

    QStringList arguments;

    QString cadexec=editor_latex();
    // QString cadexec="kile";
    QString cadarg=dirtrabajo();
    cadarg.append(QDir::separator());
    cadarg+=fichero;
    cadarg+=".tex";
    arguments << adapta(cadarg);
    // arguments << "&";

    if (! myProcess->startDetached(cadexec,arguments))
       {
         delete myProcess;
         return 1;
       }

    delete myProcess;

  return 0;
}

int editalatex_abs(QString qfichero)
{
    if (filtrartexaxtivado()) filtratex(qfichero);

    QString fichero=qfichero;

    QObject *parent=NULL;
    QProcess *myProcess = new QProcess(parent);

    QStringList arguments;

    QString cadexec=editor_latex();
    // QString cadexec="kile";
    QString cadarg=fichero;
    arguments << adapta(cadarg);
    // arguments << "&";

    if (! myProcess->startDetached(cadexec,arguments))
       {
         delete myProcess;
         return 1;
       }

    delete myProcess;

  return 0;
}


bool carga_saft_plan(QString qfichero)
{

    QDomDocument doc("AuditFile");
    QFile fichero(adapta(qfichero));
    if (!fichero.exists()) return false;

    if ( !fichero.open( QIODevice::ReadOnly ) ) return false;

    if (!doc.setContent(&fichero)) {
       fichero.close();
       return false;
      }
   fichero.close();

   QStringList codigos;
   QStringList descrip;
   QList<bool> auxiliar;

   QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        bool masterfiles=false;
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {
            if (e.tagName()=="MasterFiles")
               {
                masterfiles=true;
                //QMessageBox::warning( 0, QObject::tr("CÓDIGOS"),e.tagName());
                QDomNode jj=e.firstChild();
                while (!jj.isNull())
                   {
                    QDomElement aa = jj.toElement();
                    // QMessageBox::warning( 0, QObject::tr("CÓDIGOS"),aa.tagName());
                    if (aa.tagName()=="GeneralLedger")
                      {
                       QDomNode nn=aa.firstChild();
                       while (!nn.isNull())
                          {
                           QDomElement ee = nn.toElement();
                           if(!ee.isNull()) {
                             if (ee.tagName()=="AccountID")
                              {
                               QDomNode nnn=ee.firstChild();
                               QDomText t = nnn.toText();
                               // QMessageBox::warning( 0, QObject::tr("CÓDIGOS"),t.data());
                               codigos << t.data();
                               auxiliar << false;
                              }
                             if (ee.tagName()=="AccountDescription")
                              {
                               QDomNode nnn=ee.firstChild();
                               QDomText t = nnn.toText();
                               descrip << t.data();
                               // QMessageBox::warning( 0, QObject::tr("CÓDIGOS"),t.data());
                              }

                           }
                           nn=nn.nextSibling();
                          }
                        }
                     jj=jj.nextSibling();
                    }
                }
        }
        if (masterfiles) break;
        n = n.nextSibling();
    }

    // creamos una replica de codigos para su ordenación
    QStringList codigosrep;
    codigosrep=codigos;
    codigosrep.sort();

    for (int i=0; i< codigosrep.size(); ++i)
      {
        if (i==codigosrep.size()-1)
          {
            auxiliar[i]=true;
            break;
          }
        // vemos si el codigo actual está contenido en el siguiente
        if (!codigosrep.at(i+1).startsWith(codigosrep.at(i))) auxiliar[i]=true;
      }


    // ahora falta buscar las descripciones correctas
    // y asignarlas a descriprep
    QStringList descriprep;
    for (int i=0; i< codigosrep.size(); ++i)
      {
        QString buscar=codigosrep.at(i);
        for (int veces=0; veces<codigos.size(); ++veces)
           {
            if (codigos.at(veces)==buscar)
              {
                descriprep << descrip.at(veces);
                break;
              }
           }
      }

    // tenemos las listas codigosrep, descriprep, auxiliar con la información
    // necesaria para la importación
    QProgressDialog progreso(QObject::tr("Insertando registros ..."), 0, 0, codigosrep.size());
    progreso.setWindowModality(Qt::WindowModal);

    for (int veces=0; veces<codigosrep.size(); ++veces)
      {
        QString descripcion;
        if (existecodigoplan(codigosrep.at(veces),&descripcion)) continue;
        if (codigosrep.at(veces).length()==0) continue;
        basedatos::instancia()->insertPlancontable(codigosrep.at(veces),
                                                   descriprep.at(veces),
                                                   auxiliar.at(veces));
        progreso.setValue(veces);
        // QApplication::processEvents();
      }

   return true;
}

bool carga_saft_asientos(QString qfichero, QString usuario)
{
    // Habría que cargar primero el asiento de apertura a partir de la info de cuentas
    QDomDocument doc("AuditFile");
    QFile fichero(adapta(qfichero));
    if (!fichero.exists()) return false;

    if ( !fichero.open( QIODevice::ReadOnly ) ) return false;

    if (!doc.setContent(&fichero)) {
       fichero.close();
       return false;
      }
   fichero.close();

   QStringList codigos;
   QStringList debe;
   QStringList haber;
   QDate fecha_apertura;

   QDomElement docElem = doc.documentElement();

    QDomNode n = docElem.firstChild();
    while(!n.isNull()) {
        bool masterfiles=false;
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if(!e.isNull()) {
            if (e.tagName()=="Header")
              {
                QDomNode jj=e.firstChild();
                while (!jj.isNull())
                   {
                    QDomElement aa = jj.toElement();
                    // QMessageBox::warning( 0, QObject::tr("CÓDIGOS"),aa.tagName());
                    if (aa.tagName()=="StartDate")
                       {
                        QDomNode nnn=aa.firstChild();
                        QDomText t = nnn.toText();
                        fecha_apertura=QDate::fromString (t.data(), "yyyy-MM-dd");
                        // QMessageBox::warning( 0, QObject::tr("CÓDIGOS"),t.data());
                       }
                    jj=jj.nextSibling();
                   }
              }
            if (e.tagName()=="MasterFiles")
               {
                masterfiles=true;
                //QMessageBox::warning( 0, QObject::tr("CÓDIGOS"),e.tagName());
                QDomNode jj=e.firstChild();
                while (!jj.isNull())
                   {
                    QDomElement aa = jj.toElement();
                    // QMessageBox::warning( 0, QObject::tr("CÓDIGOS"),aa.tagName());
                    if (aa.tagName()=="GeneralLedger")
                      {
                       QDomNode nn=aa.firstChild();
                       while (!nn.isNull())
                          {
                           QDomElement ee = nn.toElement();
                           if(!ee.isNull()) {
                             if (ee.tagName()=="AccountID")
                              {
                               QDomNode nnn=ee.firstChild();
                               QDomText t = nnn.toText();
                               // QMessageBox::warning( 0, QObject::tr("CÓDIGOS"),t.data());
                               codigos << t.data();
                              }
                             if (ee.tagName()=="OpeningDebitBalance")
                              {
                               QDomNode nnn=ee.firstChild();
                               QDomText t = nnn.toText();
                               debe << t.data();
                               // QMessageBox::warning( 0, QObject::tr("CÓDIGOS"),t.data());
                              }
                             if (ee.tagName()=="OpeningCreditBalance")
                              {
                               QDomNode nnn=ee.firstChild();
                               QDomText t = nnn.toText();
                               haber << t.data();
                               // QMessageBox::warning( 0, QObject::tr("CÓDIGOS"),t.data());
                              }

                           }
                           nn=nn.nextSibling();
                          }
                        }
                     jj=jj.nextSibling();
                    }
                }
        }
        if (masterfiles) break;
        n = n.nextSibling();
    }


    // tenemos las listas codigosrep, debe, haber con la información
    // necesaria para la importación
    QProgressDialog progreso(QObject::tr("Insertando asiento apertura ..."), 0, 0, codigos.size()-1);
    progreso.setWindowModality(Qt::WindowModal);

    QString qejercicio=ejerciciodelafecha(fecha_apertura);
    if (sobreescribeproxasiento(qejercicio))
      {
        QMessageBox::warning( 0, QObject::tr("Importar asientos"),
                              QObject::tr("Error, no se pueden sobreescribir asientos\n"
                               "revise número de próximo asiento en configuración."));
       return false;
      }

    qlonglong vnum = basedatos::instancia()->proximoasiento(qejercicio);
    QString cadnum; cadnum.setNum(vnum+1);
    QString cadnumasiento;
    cadnumasiento.setNum(vnum);

    basedatos::instancia()->update_ejercicio_prox_asiento(qejercicio, cadnum);
    // procesamos asiento de apertura
    for (int veces=0; veces<codigos.size(); ++veces)
      {
        if ((debe.at(veces).toDouble()<0.001 && debe.at(veces).toDouble()>-0.001) &&
            (haber.at(veces).toDouble()<0.001 && haber.at(veces).toDouble()>-0.001))
            continue;
        if (!existesubcuenta(codigos.at(veces)))
           {
            continue;
           }

        // procesamos aquí adición de pase
        QString cadnumpase;
        qlonglong pase = basedatos::instancia()->selectProx_paseconfiguracion();
        cadnumpase.setNum(pase);
        basedatos::instancia()->insertpegaDiario(cadnumasiento, pase,
                                                 fecha_apertura.toString("yyyy-MM-dd"),
                                                 codigos.at(veces),
                                                 debe.at(veces).toDouble(),
                                                 haber.at(veces).toDouble(),
                                                 QObject::tr("Asiento de apertura"),
                                                 "","",usuario,qejercicio);

        basedatos::instancia()->updateSaldossubcuentasaldomenosmascodigo(
                haber.at(veces), debe.at(veces), codigos.at(veces));
        basedatos::instancia()->updateDiariodiarioasiento(diario_apertura(),cadnumasiento,qejercicio);
        pase++;
        cadnumpase.setNum(pase);

        basedatos::instancia()->updateConfiguracionprox_pase(cadnumpase);


        progreso.setValue(veces);
        // QApplication::processEvents();
      }
     // QApplication::processEvents();
     progreso.close();

     n = docElem.firstChild();
     int entradas=0;
     while(!n.isNull()) {
         QDomElement e = n.toElement(); // try to convert the node to an element.
         if(!e.isNull()) {
             if (e.tagName()=="GeneralLedgerEntries")
             {
               QProgressDialog progres(QObject::tr("Insertando asientos ..."), 0, 0, entradas);
               progres.setWindowModality(Qt::WindowModal);
               progres.open();
               int valprog=1;
               QDomNode jj=e.firstChild();
               while (!jj.isNull())
                    {
                      QDomElement aa = jj.toElement();
                      if (aa.tagName()=="NumberOfEntries")
                         {
                          QDomNode ccc=aa.firstChild();
                          QDomText t = ccc.toText();
                          QString cadentradas=t.data();
                          entradas=cadentradas.toInt();
                         }
                      if (aa.tagName()=="Journal")
                        {
                          QDomNode nn=aa.firstChild();
                          while (!nn.isNull())
                             {
                              QDomElement ee = nn.toElement();
                              if(!ee.isNull()) {
                                if (ee.tagName()=="Transaction")
                                  {
                                    // esto representaría un asiento
                                    QDate fecha;
                                    progres.setValue(valprog++);
                                    QDomNode hh = ee.firstChild();
                                    // número de asiento
                                    qlonglong vnum=-1;
                                    QString cadnumasiento; // a utilizar
                                    QString qdescrip;
                                    while (!hh.isNull())
                                      {
                                        QDomElement ii = hh.toElement();
                                        if (!ii.isNull()) {
                                            if (ii.tagName()=="TransactionDate")
                                              {
                                               // esta es la fecha del asiento
                                                QDomNode ccc=ii.firstChild();
                                                QDomText t = ccc.toText();
                                                fecha=QDate::fromString (t.data(), "yyyy-MM-dd");
                                                if (vnum==-1) // número de asiento no asignado
                                                   {
                                                    // asignamos próx. asiento conforme a la fecha
                                                    QString ejercicio=ejerciciodelafecha(fecha);
                                                    vnum = basedatos::instancia()->proximoasiento(ejercicio);
                                                    QString cadnum; cadnum.setNum(vnum+1);
                                                    cadnumasiento.setNum(vnum);
                                                    basedatos::instancia()->update_ejercicio_prox_asiento(ejercicio,cadnum);
                                                   }
                                              }
                                            if (ii.tagName()=="Description")
                                               {
                                                // descripción
                                                QDomNode ccc = ii.firstChild();
                                                QDomText t = ccc.toText();
                                                qdescrip=t.data();
                                               }
                                            if (ii.tagName()=="Line")
                                              {
                                                // este sería un apunte del asiento
                                                QDomNode pp = ii.firstChild();
                                                QString qcuenta, qdocumento;
                                                QString qdebe, qhaber;
                                                while (!pp.isNull())
                                                  {
                                                   QDomElement jej = pp.toElement();
                                                   if (jej.tagName()=="AccountID")
                                                      {
                                                       // cuenta
                                                       QDomNode ccc = jej.firstChild();
                                                       QDomText t = ccc.toText();
                                                       qcuenta=t.data();
                                                      }
                                                   if (jej.tagName()=="SourceDocument")
                                                      {
                                                       // documento
                                                       QDomNode ccc = jej.firstChild();
                                                       QDomText t = ccc.toText();
                                                       qdocumento=t.data();
                                                      }
                                                   if (jej.tagName()=="DebitAmount")
                                                      {
                                                       // debe
                                                       QDomNode ccc = jej.firstChild();
                                                       QDomText t = ccc.toText();
                                                       qdebe=t.data();
                                                      }
                                                   if (jej.tagName()=="CreditAmount")
                                                      {
                                                       // haber
                                                       QDomNode ccc = jej.firstChild();
                                                       QDomText t = ccc.toText();
                                                       qhaber=t.data();
                                                      }
                                                   pp=pp.nextSibling();
                                                  }
                                                // aquí procesamos el apunte
                                                if (existesubcuenta(qcuenta))
                                                   {
                                                    QString cadnumpase;
                                                    qlonglong pase = basedatos::instancia()->selectProx_paseconfiguracion();
                                                    cadnumpase.setNum(pase);
                                                    basedatos::instancia()->insertpegaDiario(cadnumasiento, pase,
                                                                                         fecha.toString("yyyy-MM-dd"),
                                                                                         qcuenta,
                                                                                         qdebe.toDouble(),
                                                                                         qhaber.toDouble(),
                                                                                         qdescrip,
                                                                                         qdocumento,"",usuario,qejercicio);

                                                     basedatos::instancia()->updateSaldossubcuentasaldomenosmascodigo(
                                                        qhaber.isEmpty() ? "0" : qhaber, qdebe.isEmpty() ? "0" : qdebe, qcuenta);
                                                     pase++;
                                                     cadnumpase.setNum(pase);
                                                     basedatos::instancia()->updateConfiguracionprox_pase(cadnumpase);
                                                  }
                                              }
                                           }
                                        hh=hh.nextSibling();
                                      }
                                  }
                               }
                               nn=nn.nextSibling();
                              }
                        }
                      jj=jj.nextSibling();
                    }
              }
       }
       n=n.nextSibling();
    }
   return true;

}

double redond(double valor, int decimales)
{
    return CutAndRoundNumberToNDecimals (valor, decimales);
}

double CutAndRoundNumberToNDecimals (double dValue, int nDec)
{
    // jsala
    /*****
    double dTemp = 0, dFract = 0, dInt = 0, dRes = 0;

    dTemp = dValue*pow(10,nDec);

    dFract = modf (dTemp, &dInt);

    if (dFract >= 0.5)
    dInt++;

    dRes = dInt/pow(10,nDec);

    return dRes;
    ******/
    // jsala El punto medio lo redondea hacia arriba sobre el valor absoluto.
      double val = ((dValue < 0)?-dValue:dValue) * pow(10,nDec);
      double fVal = floor(val);
      int inc = ((val - fVal) < (0.5 - 1e-4))?0:1;
      double r = (fVal + inc)/pow(10,nDec);
      return (dValue < 0)?-r:r;
}



void fxmldoc(QString serie, QString numero)
{
    // generamos primero el xml
    QDomDocument doc("Documento");
    QDomElement root = doc.createElement("Documento");
    doc.appendChild(root);


    QSqlQuery q;
    q = basedatos::instancia()->select_cabecera_doc (serie, numero);
    QString tipo_doc, cuenta, tipo_ret, retencion, notas, pie1, pie2;
    bool con_ret=false, con_re=false;
    QDate fecha, fecha_fac, fecha_op;
    int clave=0;
    if (q.isActive())
    {
       if (q.next())
        {
         // "select serie, numero, cuenta, fecha, fecha_fac, fecha_op, "
         // "contabilizado,contabilizable, con_ret, con_re, tipo_ret, retencion, "
         // "tipo_doc, notas, pie1, pie2, pase_diario_cta, clave "
        cuenta=q.value(2).toString();
        fecha=q.value(3).toDate();
        fecha_fac=q.value(4).toDate();
        fecha_op=q.value(5).toDate();
        con_ret=q.value(8).toBool();
        con_re=q.value(9).toBool();
        tipo_ret=q.value(10).toString();
        retencion=q.value(11).toString();
        tipo_doc=q.value(12).toString();
        notas=q.value(13).toString();
        pie1=q.value(14).toString();
        pie2=q.value(15).toString();
        clave=q.value(17).toInt();
        } else return;
    }

    QString qnombre_empresa, qdomicilio, qcp, qpoblacion;
    QString qprovincia, qpais_emisor, qcif, qid_registral;
    basedatos::instancia()->datos_empresa_tipo_doc(tipo_doc,
                                &qnombre_empresa,
                                &qdomicilio,
                                &qcp,
                                &qpoblacion,
                                &qprovincia,
                                &qpais_emisor,
                                &qcif,
                                &qid_registral
                                    );

    QDomElement tag = doc.createElement("Emisor");
    root.appendChild(tag);
    addElementoTextoDom(doc,tag,"NombreEmpresa",
                        filtracadxml(qnombre_empresa));
    addElementoTextoDom(doc,tag,"Domicilio",filtracadxml(qdomicilio));
    addElementoTextoDom(doc,tag,"Poblacion",filtracadxml(qpoblacion));
    addElementoTextoDom(doc,tag,"CP",filtracadxml(qcp));
    addElementoTextoDom(doc,tag,"Provincia",filtracadxml(qprovincia));
    addElementoTextoDom(doc,tag,"CIF",filtracadxml(qcif));
    addElementoTextoDom(doc,tag,"Pais",filtracadxml(qpais_emisor));
    addElementoTextoDom(doc,tag,"ID_REGISTRAL",filtracadxml(qid_registral));






    QString documento, cantidad, referencia, descripref, precio, totallin, msbi;
    QString mstipoiva, mstipore, mscuota, mscuotare, totallineas, totalfac, msnotas, venci;
    QString cif_empresa, cif_cliente, msnumero, msfecha, mscliente, msdescuento;
    basedatos::instancia()->msjs_tipo_doc(tipo_doc,
                                   &documento,
                                   &cantidad,
                                   &referencia,
                                   &descripref,
                                   &precio,
                                   &totallin,
                                   &msbi,
                                   &mstipoiva,
                                   &mstipore,
                                   &mscuota,
                                   &mscuotare,
                                   &totallineas,
                                   &totalfac,
                                   &msnotas,
                                   &venci,
                                   &cif_empresa,
                                   &cif_cliente,
                                   &msnumero,
                                   &msfecha,
                                   &mscliente,
                                   &msdescuento);
    // añadimos mensajes de los documentos al archivo xml
    QDomElement tag11 = doc.createElement("Etiquetas");
    root.appendChild(tag11);
    addElementoTextoDom(doc,tag11,"MSJ_Documento",filtracadxml(documento));
    addElementoTextoDom(doc,tag11,"MSJ_Cantidad",filtracadxml(cantidad));
    addElementoTextoDom(doc,tag11,"MSJ_Referencia",filtracadxml(referencia));
    addElementoTextoDom(doc,tag11,"MSJ_Descrip",filtracadxml(descripref));
    addElementoTextoDom(doc,tag11,"MSJ_Precio",filtracadxml(precio));
    addElementoTextoDom(doc,tag11,"MSJ_Total_linea",filtracadxml(totallin));
    addElementoTextoDom(doc,tag11,"MSJ_BI",filtracadxml(msbi));
    addElementoTextoDom(doc,tag11,"MSJ_Tipoiva",filtracadxml(mstipoiva));
    addElementoTextoDom(doc,tag11,"MSJ_Tipore",filtracadxml(mstipore));
    addElementoTextoDom(doc,tag11,"MSJ_Cuota",filtracadxml(mscuota));
    addElementoTextoDom(doc,tag11,"MSJ_Cuotare",filtracadxml(mscuotare));
    addElementoTextoDom(doc,tag11,"MSJ_Total_factura",filtracadxml(totalfac));
    addElementoTextoDom(doc,tag11,"MSJ_Total_lineas",filtracadxml(totallineas));
    addElementoTextoDom(doc,tag11,"MSJ_Notas",filtracadxml(msnotas));
    addElementoTextoDom(doc,tag11,"MSJ_Venci",filtracadxml(venci));
    addElementoTextoDom(doc,tag11,"MSJ_CIF_Empresa",filtracadxml(cif_empresa));
    addElementoTextoDom(doc,tag11,"MSJ_CIF_Cliente",filtracadxml(cif_cliente));
    addElementoTextoDom(doc,tag11,"MSJ_Numero",filtracadxml(msnumero));
    addElementoTextoDom(doc,tag11,"MSJ_Fecha",filtracadxml(msfecha));
    addElementoTextoDom(doc,tag11,"MSJ_Cliente",filtracadxml(mscliente));
    addElementoTextoDom(doc,tag11,"MSJ_Descuento",filtracadxml(msdescuento));

    // --------------------------------------------------


    QDomElement tag2 = doc.createElement("Datos_cabecera");
    root.appendChild(tag2);
    addElementoTextoDom(doc,tag2,"Serie",filtracadxml(serie));
    addElementoTextoDom(doc,tag2,"Numero",filtracadxml(numero));
    addElementoTextoDom(doc,tag2,"Fecha_fac",filtracadxml(fecha_fac.toString("dd/MM/yyyy")));
    addElementoTextoDom(doc,tag2,"Fecha_op",filtracadxml(fecha_op.toString("dd/MM/yyyy")));

    // QString imagen=basedatos::instancia()->imagendoc(tipo_doc);
    QString descrip_doc;
    basedatos::instancia()->existecodigotipos_doc (tipo_doc,&descrip_doc);


    addElementoTextoDom(doc,tag2,"Tipo_doc",filtracadxml(tipo_doc));


    // obtenemos información de la cuenta cliente, para los datos de la factura
    q=basedatos::instancia()->selectTodoDatossubcuentacuenta (cuenta);
    // "SELECT cuenta,razon,nombrecomercial,cif,nifrprlegal,domicilio,poblacion,";
    // "codigopostal,provincia,pais,tfno,fax,email,observaciones,ccc,cuota, ";
    // "venciasoc,codvenci,tesoreria,ivaasoc,cuentaiva,tipoiva,conceptodiario, ";
    //  "web, claveidfiscal, procesavenci "
    QString razon,nombrecomercial,cif,domicilio,poblacion,
      codigopostal,provincia,pais,codvenci;
    if (q.isActive())
     {
       if (q.next())
        {
         razon=q.value(1).toString();
         nombrecomercial=q.value(2).toString();
         cif=q.value(3).toString();
         domicilio=q.value(5).toString();
         poblacion=q.value(6).toString();
         codigopostal=q.value(7).toString();
         provincia=q.value(8).toString();
         pais=q.value(26).toString();
         codvenci=q.value(10).toString();

        }
     }
    QDomElement tag3 = doc.createElement("Destinatario");
    root.appendChild(tag3);
    addElementoTextoDom(doc,tag3,"Nombre_comercial",filtracadxml(nombrecomercial));
    addElementoTextoDom(doc,tag3,"Razon",filtracadxml(razon));
    addElementoTextoDom(doc,tag3,"Cif",filtracadxml(cif));
    addElementoTextoDom(doc,tag3,"Domicilio",filtracadxml(domicilio));
    addElementoTextoDom(doc,tag3,"Ciudad",filtracadxml(poblacion));
    addElementoTextoDom(doc,tag3,"Codigo_postal",filtracadxml(codigopostal));
    addElementoTextoDom(doc,tag3,"Provincia",filtracadxml(provincia));
    addElementoTextoDom(doc,tag3,"Nacion",filtracadxml(pais));

    QDomElement tag4 = doc.createElement("Detalle");
    root.appendChild(tag4);


    q = basedatos::instancia()->select_lin_doc(clave);
    double totalfactura=0;
    if (q.isActive())
       while (q.next())
        {
        QDomElement tag5 = doc.createElement("Linea");
        tag4.appendChild(tag5);
        // clave, codigo, descripcion, cantidad, precio, clave_iva, tipo_iva, tipo_re
        addElementoTextoDom(doc,tag5,"Codigo",filtracadxml(q.value(1).toString()));
        addElementoTextoDom(doc,tag5,"Descrip",filtracadxml(q.value(2).toString()));
        addElementoTextoDom(doc,tag5,"Cantidad",filtracadxml(convacoma(q.value(3).toString())));
        addElementoTextoDom(doc,tag5,"Precio",filtracadxml(formateanumero(
                q.value(4).toDouble(),true,true)));
        addElementoTextoDom(doc,tag5,"Tipo_iva",filtracadxml(formateanumero(
                q.value(6).toDouble(),true,true)));
        addElementoTextoDom(doc,tag5,"Tipo_re",filtracadxml(formateanumero(
                q.value(7).toDouble(),true,true)));
        if (q.value(8).toDouble()>-0.001 && q.value(8).toDouble()<0.001)
            addElementoTextoDom(doc,tag5,"Descuento","");
          else
            addElementoTextoDom(doc,tag5,"Descuento",filtracadxml(convacoma(q.value(8).toString())));
        totalfactura+=q.value(3).toDouble()*q.value(4).toDouble()*(1-q.value(8).toDouble()/100)+
         q.value(3).toDouble()*q.value(4).toDouble()*(1-q.value(8).toDouble()/100)*q.value(6).toDouble()/100+
         q.value(3).toDouble()*q.value(4).toDouble()*(1-q.value(8).toDouble()/100)*q.value(7).toDouble()/100;
        }


    // datos del pie de la factura
    // incluiría total fra. y desgloses por tipos de IVA

    QDomElement tag6 = doc.createElement("Datos_pie");
    root.appendChild(tag6);
    addElementoTextoDom(doc,tag6,"Con_ret",con_ret ? "true" : "false");
    addElementoTextoDom(doc,tag6,"Con_re",con_re ? "true" : "false");
    addElementoTextoDom(doc,tag6,"Tipo_ret",filtracadxml(convacoma(tipo_ret)));
    addElementoTextoDom(doc,tag6,"Retencion",filtracadxml(convacoma(retencion)));
    addElementoTextoDom(doc,tag6,"Notas",filtracadxml(notas));
    addElementoTextoDom(doc,tag6,"Pie1",filtracadxml(pie1));
    addElementoTextoDom(doc,tag6,"Pie2",filtracadxml(pie2));
    // -----------------------------------------------------------------------------------
    q = basedatos::instancia()->select_iva_lin_doc (clave);
    if (q.isActive())
      {
        if (q.next())
        {
         addElementoTextoDom(doc,tag6,"Base1",convacoma(q.value(0).toString()));
         addElementoTextoDom(doc,tag6,"Tipo_iva1",convacoma(q.value(2).toString()));
         addElementoTextoDom(doc,tag6,"Tipo_re1",convacoma(q.value(3).toString()));
         addElementoTextoDom(doc,tag6,"Cuota_iva1",formateanumero(
            q.value(0).toDouble()*q.value(2).toDouble()/100,true,true));
         addElementoTextoDom(doc,tag6,"Cuota_re1",formateanumero(
            q.value(0).toDouble()*q.value(3).toDouble()/100,true,true));
        }
        else
            {
            addElementoTextoDom(doc,tag6,"Base1","");
            addElementoTextoDom(doc,tag6,"Tipo_iva1","");
            addElementoTextoDom(doc,tag6,"Tipo_re1","");
            addElementoTextoDom(doc,tag6,"Cuota_iva1","");
            addElementoTextoDom(doc,tag6,"Cuota_re1","");
            }
        if (q.next())
        {
         addElementoTextoDom(doc,tag6,"Base2",convacoma(q.value(0).toString()));
         addElementoTextoDom(doc,tag6,"Tipo_iva2",convacoma(q.value(2).toString()));
         addElementoTextoDom(doc,tag6,"Tipo_re2",convacoma(q.value(3).toString()));
         addElementoTextoDom(doc,tag6,"Cuota_iva2",formateanumero(
            q.value(0).toDouble()*q.value(2).toDouble()/100,true,true));
         addElementoTextoDom(doc,tag6,"Cuota_re2",formateanumero(
            q.value(0).toDouble()*q.value(3).toDouble()/100,true,true));
        }
        else
            {
            addElementoTextoDom(doc,tag6,"Base2","");
            addElementoTextoDom(doc,tag6,"Tipo_iva2","");
            addElementoTextoDom(doc,tag6,"Tipo_re2","");
            addElementoTextoDom(doc,tag6,"Cuota_iva2","");
            addElementoTextoDom(doc,tag6,"Cuota_re2","");
            }
        if (q.next())
        {
         addElementoTextoDom(doc,tag6,"Base3",convacoma(q.value(0).toString()));
         addElementoTextoDom(doc,tag6,"Tipo_iva3",convacoma(q.value(2).toString()));
         addElementoTextoDom(doc,tag6,"Tipo_re3",convacoma(q.value(3).toString()));
         addElementoTextoDom(doc,tag6,"Cuota_iva3",formateanumero(
            q.value(0).toDouble()*q.value(2).toDouble()/100,true,true));
         addElementoTextoDom(doc,tag6,"Cuota_re3",formateanumero(
            q.value(0).toDouble()*q.value(3).toDouble()/100,true,true));
        }
        else
            {
            addElementoTextoDom(doc,tag6,"Base3","");
            addElementoTextoDom(doc,tag6,"Tipo_iva3","");
            addElementoTextoDom(doc,tag6,"Tipo_re3","");
            addElementoTextoDom(doc,tag6,"Cuota_iva3","");
            addElementoTextoDom(doc,tag6,"Cuota_re3","");
            }
      }
    // -----------------------------------------------------------------------------------
    /*QDomElement tag7 = doc.createElement("Desglose_iva");
    tag6.appendChild(tag7);

    q = basedatos::instancia()->select_iva_lin_doc (clave);
    if (q.isActive())
       while (q.next())
        {
         // "sum(cantidad*precio*(1-dto/100)), "
         // "clave_iva, max(tipo_iva), max(tipo_re) "
        addElementoTextoDom(doc,tag7,"Codigo",filtracadxml(q.value(1).toString()));
        addElementoTextoDom(doc,tag7,"Base",q.value(0).toString());
        addElementoTextoDom(doc,tag7,"Tipo_iva",q.value(2).toString());
        addElementoTextoDom(doc,tag7,"Tipo_re",q.value(3).toString());
        addElementoTextoDom(doc,tag7,"Cuota_iva",formateanumero(
                q.value(0).toDouble()*q.value(2).toDouble()/100,false,true));
        addElementoTextoDom(doc,tag7,"Cuota_re",formateanumero(
                q.value(0).toDouble()*q.value(3).toDouble()/100,false,true));
        }
    */
    totalfactura=totalfactura-retencion.toDouble();
    addElementoTextoDom(doc,tag6,"Total",formateanumero(totalfactura,true,true));

    QString xml = doc.toString();
    // -----------------------------------------------------------------------------------
    QString qfichero=dirtrabajodocs(tipo_doc); // el directorio será dirtrabajo() + ruta tipo doc
    qfichero.append(QDir::separator());
    QString cadfich=serie+numero+".xml";
    QString rutagraf=qfichero+tipo_doc+".png";
    qfichero=qfichero+cadfich;

    QFile fichero( adapta(qfichero)  );

    if ( !fichero.open( QIODevice::WriteOnly ) ) return;

    QTextStream stream( &fichero );
    stream.setCodec("UTF-8");

    stream << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    stream << "<?xml-stylesheet type=\"text/xsl\" href=\"automat2html.xslt\"?>\n";

    // bool esmysql = ( basedatos::instancia()->cualControlador() == basedatos::MYSQL );

    // ------------------------------------------------------------------------------------
    stream << xml;
    fichero.close();

    // generamos imagen del logo
   /* QPixmap foto;
    if (imagen.length()>0)
          {
           QByteArray byteshexa;
           byteshexa.append ( imagen );
           QByteArray bytes;
           bytes=bytes.fromHex ( byteshexa );
           foto.loadFromData ( bytes, "PNG");
           foto.save(rutagraf,"PNG");
          }*/

}


QString latex_doc(QString serie, QString numero)
{
    bool coma=haycomadecimal();
    bool decimales=haydecimales();

    QString nombre_empresa=basedatos::instancia()->selectEmpresaconfiguracion();
    QString domicilio=basedatos::instancia()->domicilio();
    QString poblacion=basedatos::instancia()->ciudad();
    QString cp=basedatos::instancia()->cpostal();
    QString provincia=basedatos::instancia()->provincia();
    QString cif=basedatos::instancia()->cif();
    QString id_registral=basedatos::instancia()->idregistral();
    QString pais_emisor;

    QSqlQuery q;
    q = basedatos::instancia()->select_cabecera_doc (serie, numero);
    QString tipo_doc, cuenta, tipo_ret, retencion, notas, pie1, pie2;
    bool con_ret=false, con_re=false;
    QDate fecha, fecha_fac, fecha_op;
    int clave=0;
    if (q.isActive())
    {
       if (q.next())
        {
         // "select serie, numero, cuenta, fecha, fecha_fac, fecha_op, "
         // "contabilizado,contabilizable, con_ret, con_re, tipo_ret, retencion, "
         // "tipo_doc, notas, pie1, pie2, pase_diario_cta, clave "
        cuenta=q.value(2).toString();
        fecha=q.value(3).toDate();
        fecha_fac=q.value(4).toDate();
        fecha_op=q.value(5).toDate();
        con_ret=q.value(8).toBool();
        con_re=q.value(9).toBool();
        tipo_ret=q.value(10).toString();
        retencion=q.value(11).toString();
        tipo_doc=q.value(12).toString();
        notas=q.value(13).toString();
        pie1=q.value(14).toString();
        pie2=q.value(15).toString();
        clave=q.value(17).toInt();
        } else return QString();
    }

    basedatos::instancia()->datos_empresa_tipo_doc(tipo_doc,
                                &nombre_empresa,
                                &domicilio,
                                &cp,
                                &poblacion,
                                &provincia,
                                &pais_emisor,
                                &cif,
                                &id_registral
                                    );


    int tipo_operacion=basedatos::instancia()->tipo_operacion_tipo_doc(tipo_doc);

    QString documento, cantidad, referencia, descripref, precio, totallin, totallineas, msbi;
    QString mstipoiva, mstipore, mscuota, mscuotare, totalfac, msnotas, venci;
    QString mscif_empresa, mscif_cliente, msnumero, msfecha, mscliente, msdescuento ;
    basedatos::instancia()->msjs_tipo_doc(tipo_doc,
                                   &documento,
                                   &cantidad,
                                   &referencia,
                                   &descripref,
                                   &precio,
                                   &totallin,
                                   &msbi,
                                   &mstipoiva,
                                   &mstipore,
                                   &mscuota,
                                   &mscuotare,
                                   &totallineas,
                                   &totalfac,
                                   &msnotas,
                                   &venci,
                                   &mscif_empresa,
                                   &mscif_cliente,
                                   &msnumero,
                                   &msfecha,
                                   &mscliente,
                                   &msdescuento);

    QString descripdoc=basedatos::instancia()->descrip_doc(tipo_doc);
    QString moneda;
    if (basedatos::instancia()->es_euro_tipo_doc(tipo_doc))
      {
        moneda="\\texteuro ";
      }
    int lineas_doc=basedatos::instancia()->lineas_doc(tipo_doc);
    // obtenemos información de la cuenta cliente, para los datos de la factura
    q=basedatos::instancia()->selectTodoDatossubcuentacuenta (cuenta);
    // "SELECT cuenta,razon,nombrecomercial,cif,nifrprlegal,domicilio,poblacion,";
    // "codigopostal,provincia,pais,tfno,fax,email,observaciones,ccc,cuota, ";
    // "venciasoc,codvenci,tesoreria,ivaasoc,cuentaiva,tipoiva,conceptodiario, ";
    //  "web, claveidfiscal, procesavenci "
    QString razon,nombrecomercial,cif2,domicilio2,poblacion2,
      codigopostal,provincia2,pais,codvenci;
    if (q.isActive())
     {
       if (q.next())
        {
         razon=q.value(1).toString();
         nombrecomercial=q.value(2).toString();
         cif2=q.value(3).toString();
         domicilio2=q.value(5).toString();
         poblacion2=q.value(6).toString();
         codigopostal=q.value(7).toString();
         provincia2=q.value(8).toString();
         pais=q.value(26).toString();
         codvenci=q.value(10).toString();

        }
     }


    QString qfichero=dirtrabajodocs(tipo_doc); // el directorio será dirtrabajo() + ruta tipo doc
    qfichero.append(QDir::separator());
    QString rutagraf=qfichero+tipo_doc+".png";
    QString cadfich=serie+numero+".tex";
    qfichero.append(cadfich);

    // generamos imagen del logo
    QString imagen=basedatos::instancia()->imagendoc(tipo_doc);
    QPixmap foto;
    if (imagen.length()>0)
          {
           QByteArray byteshexa;
           byteshexa.append ( imagen );
           QByteArray bytes;
           bytes=bytes.fromHex ( byteshexa );
           foto.loadFromData ( bytes, "PNG");
           foto.save(rutagraf,"PNG");
          }


    QFile fichero(qfichero);
    if (! fichero.open( QIODevice::WriteOnly ) ) return QString();
    QTextStream stream( &fichero );
    stream.setCodec("UTF-8");
    stream << cabeceralatex_graficos();
    stream << margen_extra_latex();

    QString cabecera_factura; // asignar la cabecera factura
    QString pie_factura; // asignar el pie factura

    if (!imagen.isEmpty())
      {
       cabecera_factura="\\begin{figure}[t]\n";
       cabecera_factura+="\\includegraphics[width=150pt]{";
       if (WINDOWS) rutagraf=rutagraf.replace("\\","/");
       cabecera_factura+= rutagraf  +"}\n";
       cabecera_factura+="\\end{figure}\n";
      }

    cabecera_factura+="\\begin{flushright}\n";
    // cabecera_factura+="\\begin{tabular}{|c|}\n";
    // cabecera_factura+="\\hline\n";
    cabecera_factura+="{\\huge \\textbf {";
    cabecera_factura+=filtracad(documento);
    cabecera_factura+="}} \\\\ \n";
    // cabecera_factura+="\\hline\n";
    // cabecera_factura+="\\end{tabular}\n";
    cabecera_factura+="\\end{flushright}\n\n";

    cabecera_factura+="\\begin{flushleft}\n";
    if (!nombre_empresa.isEmpty())
        cabecera_factura+="\\bf {\\Large " + filtracad(nombre_empresa)+"} \\\\\n";
    if (!domicilio.isEmpty()) cabecera_factura+=filtracad(domicilio)+"\\\\\n";
    cabecera_factura+=filtracad(cp)+" - ";
    if (!poblacion.isEmpty()) cabecera_factura+=filtracad(poblacion)+"\\\\\n";
    if (!provincia.isEmpty()) cabecera_factura+=filtracad(provincia)+"\\\\\n";
    if (!pais_emisor.isEmpty()) cabecera_factura+=filtracad(pais_emisor)+"\\\\\n";
    cabecera_factura+=filtracad(mscif_empresa)+": "+cif+"\\\\\n";
    cabecera_factura+="\\end{flushleft}\n\n";

    cabecera_factura+="\\begin{flushleft}\n";
    cabecera_factura+="\\hspace{5cm}\n";
    cabecera_factura+="\\begin{tabular}{|p{8cm}|}\n";
    cabecera_factura+="\\hline\n";

    if (!razon.isEmpty()) cabecera_factura+="\\bf {\\large "+ filtracad(razon) + "} \\\\\n";
    if (!domicilio2.isEmpty()) cabecera_factura+=filtracad(domicilio2) + "\\\\\n";
    cabecera_factura+=filtracad(codigopostal) + " - " + filtracad(poblacion2) + "\\\\\n";
    // qWarning(pais.toAscii().constData());
    // QString elpais=basedatos::instancia()->descrip_pais(pais);
    // qWarning(elpais.toAscii().constData());
    cabecera_factura+=filtracad(provincia2) + "\\\\\n"; // + " - " + filtracad(elpais) + "\\\\\n";
    if (!pais.isEmpty()) cabecera_factura+=filtracad(pais) +  "\\\\\n";
    if (!cif2.isEmpty())
       cabecera_factura+=filtracad(mscif_cliente) + ": " + filtracad(cif2) + "\\\\\n";
    cabecera_factura+="\\hline\n";
    cabecera_factura+="\\end{tabular}\n";
    cabecera_factura+="\\end{flushleft}\n\n";
    cabecera_factura+="\\vspace{0.1cm}\n";

    // fecha y número de factura
    cabecera_factura+="\\begin{flushright}\n";
    cabecera_factura+="\\begin{tabular}{|l|r|l|r|}\n";
    cabecera_factura+="\\hline\n";
    cabecera_factura+="\\scriptsize ";
    cabecera_factura+=filtracad(msfecha)+" & \\scriptsize " +
                      fecha_fac.toString("dd-MM-yyyy") + " & \\scriptsize " ;
    cabecera_factura+=filtracad(msnumero)+" & \\scriptsize "+ serie+numero + "\\\\\n";
    cabecera_factura+="\\hline\n";
    cabecera_factura+="\\end{tabular}\n\n";
    cabecera_factura+="\\end{flushright}\n\n";
    // cabecera_factura+="\\vspace{0.1cm}\n";
    // codigo, descripcion, cantidad, precio, descuento, total
    cabecera_factura+="\\begin{center}\n";
    cabecera_factura+="\\begin{tabular}{|l|p{7cm}|r|r|r|r|}\n";
    cabecera_factura+="\\hline\n";
    // cantidad, referencia, descripref, precio, totallin
    cabecera_factura+="\\scriptsize "+ filtracad(referencia)+ "&"+
                      "\\scriptsize "+ filtracad(descripref) + "&" +
                      "\\scriptsize " + filtracad(cantidad) + "&" +
                      "\\scriptsize " + filtracad(precio) +
                      "&" + "\\scriptsize " +filtracad(msdescuento) + "&" +
                      "\\scriptsize " + filtracad(totallin) + "\\\\\n";
    cabecera_factura+="\\hline\n";

    q = basedatos::instancia()->select_lin_doc(clave);
    // double totalfactura=0;
    double sumalineas=0;
    if (q.isActive())
       while (q.next())
        {
         double suma=q.value(3).toDouble()*q.value(4).toDouble() *
                    (1-q.value(8).toDouble()/100);
         sumalineas+=suma;
        }

    // Pie de la factura
    // escribimos notas de factura e identidad registral
    pie_factura += "\\multicolumn{5}{|r|}{\\scriptsize ";
    pie_factura += totallineas + "} & "+
              "\\scriptsize " + formateanumerosep(sumalineas,coma,decimales) + moneda + "\\\\\n";
    pie_factura +="\\hline\n";
    pie_factura +="\\end{tabular}\n";
    pie_factura+="\\end{center}\n\n";
    double totalfactura=sumalineas;
    if (tipo_operacion==1) // operacion convencional
       {
        // base imponible, tipo, cuota, precio, descuento, total
        // pie_factura +="\\vspace{0.1cm}\n";
        pie_factura+="\\begin{flushright}\n";
        if (con_re) pie_factura += "\\begin{tabular}{|r|r|r|r|r|}\n";
           else pie_factura += "\\begin{tabular}{|r|r|r|}\n";
        pie_factura += "\\hline\n";
        // cantidad, referencia, descripref, precio, totallin
        if (con_re)
          pie_factura += "\\footnotesize " + filtracad(msbi) + "&"+
                         "\\footnotesize " + filtracad(mstipoiva)
                         + "&" + "\\footnotesize " + filtracad(mstipore) + "&"
                         + "\\footnotesize " +filtracad(mscuota) + " & " +
                         "\\footnotesize " + filtracad(mscuotare) + "\\\\\n";
          else
              pie_factura += "\\footnotesize " + filtracad(msbi) + "&"+
                             "\\footnotesize " +filtracad(mstipoiva)
                             + "&"
                             + "\\footnotesize " + filtracad(mscuota) + "\\\\\n";
        pie_factura += "\\hline\n";
        q = basedatos::instancia()->select_iva_lin_doc (clave);
        if (q.isActive())
            while (q.next())
             {
             // "sum(cantidad*precio*(1-dto/100)), "
             // "clave_iva, max(tipo_iva), max(tipo_re) "
             pie_factura  += "\\footnotesize " + formateanumerosep(q.value(0).toDouble(),coma,decimales);
             pie_factura += " & ";
             pie_factura  += "\\footnotesize " + formateanumerosep(q.value(2).toDouble(),coma,decimales);
             pie_factura += " & ";
             if (con_re)
               {
                pie_factura  += "\\footnotesize " + formateanumerosep(q.value(3).toDouble(),coma,decimales);
                pie_factura += " & ";
               }
             pie_factura  += "\\footnotesize " + formateanumerosep(q.value(0).toDouble()*q.value(2).toDouble()/100
                                              ,coma,decimales);
             totalfactura+=q.value(0).toDouble()*q.value(2).toDouble()/100;
             if (con_re)
               {
                pie_factura += " & ";
                pie_factura  += "\\footnotesize " + formateanumerosep(q.value(0).toDouble()*q.value(3).toDouble()/100,
                                              coma,decimales);
                totalfactura+=q.value(0).toDouble()*q.value(3).toDouble()/100;
               }
             pie_factura += "\\\\\\hline\n";
            }
        pie_factura +="\\end{tabular}\n";
        pie_factura+="\\end{flushright}\n";
       }

    // retención y total factura
    // con_ret
    // tipo_ret
    // retencion
    //pie_factura +="\\vspace{0.1cm}\n";
    pie_factura+="\\begin{flushright}\n";
    pie_factura += "\\begin{tabular}{|r|r|}\n";
    pie_factura += "\\hline\n";
    if (con_ret)
      {
        pie_factura += "\\footnotesize " + QObject::tr("Retención IRPF (");
        pie_factura += tipo_ret;
        pie_factura += "\\%)";
        pie_factura += " & ";
        pie_factura += "\\footnotesize " + formateanumerosep(retencion.toDouble(),coma,decimales);
        pie_factura += "\\\\\\hline\n";
        totalfactura -= retencion.toDouble();
      }
    pie_factura += "\\footnotesize " + totalfac;
    pie_factura += " & ";
    pie_factura += "\\footnotesize " + formateanumerosep(totalfactura,coma,decimales);
    pie_factura += moneda + "\\\\\\hline\n";
    pie_factura +="\\end{tabular}\n";
    pie_factura+="\\end{flushright}\n\n";

    if (!notas.isEmpty())
     {
      pie_factura+="\\begin{center}\n";
      pie_factura+="\\begin{tabular}{|p{15cm}|}\n";
      pie_factura+="\\hline\n";
      pie_factura+="\\scriptsize " + notas+"\\\\\n\n";
      pie_factura +="\\hline\n";
      pie_factura +="\\end{tabular}\n";
      pie_factura +="\\end{center}\n";
     }
    if (!pie1.isEmpty()) pie_factura+="\\scriptsize " + pie1+"\\\\\n";
    if (!pie2.isEmpty()) pie_factura+="\\scriptsize " + pie2+"\n\n";
    pie_factura+="\\begin{center}\n";
    pie_factura+="\\tiny " + id_registral+"\n\n";
    pie_factura+="\\end{center}\n";
    // repetimos para imprimir cuadro de la factura
    int linea=1;
    q = basedatos::instancia()->select_lin_doc(clave);
    if (q.isActive())
       while (q.next())
        {
        if (linea==1) stream << cabecera_factura;
        stream << "\\scriptsize " << filtracad(q.value(1).toString()); // codigo
        stream << " & ";
        stream << "\\scriptsize " << filtracad(q.value(2).toString()); // descripción
        stream << " & ";
        QString qcantidad=q.value(3).toString();
        if (qcantidad.contains(".000")) qcantidad.remove(".000");

        stream << "\\scriptsize " << convacoma(qcantidad); // cantidad
        stream << " & ";
        stream << "\\scriptsize " << formateanumerosep(q.value(4).toDouble(),coma,decimales); // precio
        stream << " & ";
        stream << "\\scriptsize " << (q.value(8).toDouble() !=0 ? q.value(8).toString(): ""); // descuento
        stream << " & ";
        stream << "\\scriptsize " << formateanumerosep(q.value(3).toDouble()*q.value(4).toDouble()*
                                    (1-q.value(8).toDouble()/100),coma,
                                    decimales);
        stream << "\\\\";
        stream << "\\hline\n";
        linea++;
        if (linea>lineas_doc)
           {
            // pie cuadro y factura
            linea=1;
            stream << pie_factura;
            // nueva página
            stream << "\\newpage\n";
           }
        }

    if (linea==lineas_doc) stream << pie_factura;

    if (linea<lineas_doc && linea>1)
       {
         // completamos cuadro fra. y pie
        for (int veces=linea; veces<lineas_doc; veces++)
           {
            stream << " & & & & & \\\\";
            stream << "\\hline\n";
           }
        stream << pie_factura;
       }

    // imprimimos final del documento latex
    stream << "% FIN_CUERPO" << "\n";
    stream << "\\end{document}" << "\n";

    fichero.close();

  return qfichero;
}


double total_documento(QString serie, QString numero)
{
    QSqlQuery q;
    q = basedatos::instancia()->select_cabecera_doc (serie, numero);
    QString tipo_ret, retencion;
    bool con_ret=false, con_re=false;
    int clave=0;
    double totalfactura=0;
    if (q.isActive())
    {
       if (q.next())
        {
         // "select serie, numero, cuenta, fecha, fecha_fac, fecha_op, "
         // "contabilizado,contabilizable, con_ret, con_re, tipo_ret, retencion, "
         // "tipo_doc, notas, pie1, pie2, pase_diario_cta, clave "
        con_ret=q.value(8).toBool();
        con_re=q.value(9).toBool();
        tipo_ret=q.value(10).toString();
        retencion=q.value(11).toString();
        clave=q.value(17).toInt();
        } else return 0;
    }
    // int tipo_operacion=basedatos::instancia()->tipo_operacion_tipo_doc(tipo_doc);
    q = basedatos::instancia()->select_lin_doc(clave);
    double sumalineas=0;
    if (q.isActive())
       while (q.next())
        {
         double suma=q.value(3).toDouble()*q.value(4).toDouble() *
                    (1-q.value(8).toDouble()/100);
         sumalineas+=suma;
        }

    totalfactura+=sumalineas;
    q = basedatos::instancia()->select_iva_lin_doc (clave);
    if (q.isActive())
        while (q.next())
         {
         // "sum(cantidad*precio*(1-dto/100)), "
         // "clave_iva, max(tipo_iva), max(tipo_re) "
         // QMessageBox::warning( 0, QObject::tr("iva"),q.value(0).toString());

         totalfactura+=q.value(0).toDouble()*q.value(2).toDouble()/100;
         if (con_re)
            totalfactura+=q.value(0).toDouble()*q.value(3).toDouble()/100;
        }

    if (con_ret)
        totalfactura -= retencion.toDouble();

    return totalfactura;

}


bool genera_pdf_latex(QString qfichero)
{
    if (filtrartexaxtivado()) filtratex(qfichero);

    QString fichero=qfichero;
    // fichero.truncate(qfichero.length()-4);
    QString cadarg=fichero;

    QObject *parent=NULL;

     QStringList arguments;
     arguments << adapta(cadarg);

     QString cadexec="pdflatex";
     QProcess *myProcess = new QProcess(parent);

     int pos=qfichero.lastIndexOf(QDir::separator());
     QString directorio=qfichero.left(pos);

     myProcess-> setWorkingDirectory(adapta(directorio));

     myProcess->start(cadexec,arguments);

    // system(cadexec);
    if (!myProcess->waitForStarted ())
       {
         delete myProcess;
         return false;
       }
    if (!myProcess->waitForFinished ())
       {
         delete myProcess;
         return false;
       }

  return true;
}

QString tipo_asiento_general()
{
    return QObject::tr("GENERAL");
}


QString logodefecto()
{
#ifndef SMART
    QString logo=
            "89504e470d0a1a0a0000000d49484452000000c8000000640806000000c3867f0b0000000473424954080808087c086488000000097048597300000ec400000ec401952b0e1b0000200049444154789ced9d79985d4775e07fa7eef696ded52da95bbb6dc996bc5b5ed9bc83c166f908185b26ac01b36799cce4830c43cc24131892219021c43343088b6530216043cc8e31de3136606cc996655b9b25abd5ddeaed2d77a93af3c77d2db76549d6d2dd12e2fdbeaff5debb5b555ddd73eb9c3aa74e4193264d9a3469d2a44993264d9a3469d2a44993264d9a34f91d460e47a15e47cfa78f59d073c9294b7a07abb5ea9d61a9e5be9b6ffed1770e475d9a34d917332e201df316fefd5845ffcc6631a7f57532bb43500950356821fab56fa2bf72c1c88f7ff4a3872a335db7264d76c79be9027b17ccfff8d86832dff94635cba43df449ea31699c4292ce8dabe3572531971db364e1d0c62ddbd6cc74fd9a34998c99e9022df13b4c98405a919ebe9460ce08fe9c0ae1dc51cadd55162deed2ee96f219d5816dab5f7cde295f99e9fa35693299c362835cf6c1cba28561c769675db862f45d57fcb7b500af7afdf2459578e0335ebdfa9a59618ff4cd3959efbb7fad4473bbeebbfd27bf38f770d4b34993238e375c7dd2b5e79d18565e77f17c7dff9b2e75271dbb502f78e565ab5575c6d5c1264d8eb8876ecdc3fd0f9c71465fe5998dfd972101c60f65d3334373bff5fd9fdebaf1f147fb0f77fd9afc7e71c40908c0634f8cde77fce2596f7ba6ff994e713e552be5f1b1f1fb86766cffcde1ae5b93df2f66dc48df5fea63fa3f3deb514d1334b314cba5771eee3a35f9fde3881590aa4bd71425c85495cc2a56d397fde5dffddd82c35daf26bf5f1c9902a2484a6d8be7a79b8320c36946bd52c35affd489fdaa7a5846e09afc7e71243d64eddfba6dc59265bd3b176d1b4e2f366978d257fea67af6ba7e537e7a244042e1acab666f8bd5aef3d2f2605b61d6401cebcd0b671dbfe6937ffa990d87bbf24d8e4e8e0801b9f927c75c74ec92faa77a7b46161603e90e439fc11d70db1795b5eb859b7e912038ed7e7151321cc68578c6a714ce22088b1b4be1ecaf7eed6ffee3bf1eee763439fa38ac2ad6719711fdeb7fccfae2252feafffe8a8583677496e88e0241ac8f140bfcf0d18495a7858459c6dc2e5ffc4a8aab2b2e4dd1c0aa295bd2acb668b4baf123577ff88a873ef64f1f9b7d38dbd3e4e8c33f5c057fe7c7e7ceebe858bbe6252be3b60cf8cacfcbd892e5152b607649682b08cfc48e0f7daa9f200a086be06915ca115e6b899882143a324c3d0327d4d87472ff58f1d66fdefecd57ffc1f97fb0ed70b5eb7785d5a013df57eda726b1b7730e74fb54d7eb403990320e8b80bcfbfa95a5b34edbb87aceaca4cd5955df0be5c253ea3cb8c5e3fead8e745ca890111d6f98ddd14e4446d8e9317b792f78303e04c9f602ae3f45d5a19e912453ed1f7f7ce56fd7dff375e0658752bf1be015026f5338179843ee2fda2ef02070cb2af89729b80d07c44c3c384d9ecf611190ebdeb4f1355da5f865d6891ae3893ac7bc76a5b75d516b402ba81770cea276fec70f1266b774d0d912b0757dcad888c2764b62c7297a1199b1d844719e1311c7631bee7de9e7ffedf36f7bef1bdefbaf075aaf9ba03d83af0257c0f39ec2058dbfd7721804a4c9e161c605e433b7124581f998ef5b448d8041c507750801bea9319215f8e1432dfc604d86898bacdb5427de129300549485cb43bacb359e78a44675cc811afcc0101522d4eed4c736dff3974343faadae2e19d9df7a7d0c4c0adf14b81840e157021fa9c25d7d900cc0020f2e00de3e3d77e6c8e7407baea3a1a79b710159de3bff5d2d85b113c489aa3122181c8221040375dbcd3fdf13f3f06683a148cd26f4f4b5336bf92cb234e1feef6ee4173fd8c915ef6c434eacf2f0830e4f044f2cced5416b0c559e5efcc5ef7fe222e05bfb5baf6570f524e178c4c24bdf0293276dad6ffcfdbfddcf5d0d572bbc5de074850e8111855f1bf897ab61f56ec7ee5295ae06b31afe54e0bd0a0b05b6087cfe2af87b691c37f9f83d5d63e221fc2abccdc017271d5607b602b765f089b7e475df231f037302fc998377018b806d02ab7df8eb2ba1b6af72f7c5de8ebf098a19fc25b04aa10fd8085cbf0a3e7de35eae75a0ed3b9832f6c44c8f62854be6d5ff3130751c82880104a3f9ac4243c4f71f4958d76f083d83b5294b972ce6c4e38fa577563bb3e7cce545972c23f24adcf793518e59516456af417d5003a94b19ab8d48b536e80f8d6ebbeab6db6e3b9017c05b26be087c7c37e1d8231f0373037c0d582d7029d02df94b6796c0c50a37dc005ffbd85eeef36af82781bf078e13088163143eb51a3e7400f59ea8f305bb6d2a00c700eff4e1811be084bd9dbb0cfe59e15302cb042281c5c04732f8def5101c685df6c5f51064f07df2877749a3bc65027f7f23fcf3dece3b90f61d6c197b624605e4f337f77ce4989e3a6a438c48e38de2216210b50c571d0f6c3318c0f73d569e720a592d264b2d10e10b04ad119d734a8c6c17aa238ebe6314df0bc053cabd0532933232b2937ab2f3ca31b676ec6fdd04ce98f89ec1cff6e79c65f01e8137357e3ea670a60f91c299c0638debbe6919bc672f97381f38d58712f0f1497579efc4f75520bbbfad27b6edb63d00dee6c3fc5e0852e85698f00db5c9b3df9f87e4831a67f9101958a9b076a27e2df0c117be13fb4fe37a2f0350586b60e5a4725fba8f53f7bb7d8750c6f3983101e9ba8cb68b56666fb256558c031154f31e041c003bc63d76c61e2e558e5bbc18dff3e9eded6370680813c0f6eddb71c42c5ed207759f917ee89913e017aae00cd64b28cf0ba9d42bba65c73afa2b6357ef6ffd147609d30218da9f7304de31e9e7fbae8107ae84e41a780078dfa47d7bb45b043eb00a1eba126a317c7ad2ae63f6b7de135c03d7ac822f5d094f5f08d95b6130804f4cecd7e7bf8127f3be55f0cb2b21b90a1e5478ffa47d6f3ed0baec0b813f9cf86ee07d57c1837b29f7391c48fb0eb68c3d316336c8673fdc7161576b729c87c985c37988084a06a2380c3b6a305aa933b7ab8d72a9852c01950cbf20acdff4242d7e0b3db3e790ce4928861ef108cc390ec2d0231eb3482a98a2c38aca5875906d039b3f0afce3fed44f6018e806d80c5dc00bce3d515831f10af7e19ec9fb7cb8277bf6e78a3d9d1fc3dd13df37c0e8f1cfee3a20b5464156c33b25d7b74f023a05fcecb987cdd9dbf9bbd73d847b279dbb57d5ec6050387ee29e7970df3eca9d7cce01b5ef60cad81b33d283bcf126bcc55dbca9a3d5f3c5cb0435888062018baa87b888e18a214b944a5551a0e465cc291598dfd5c2fc4e615eef5cc4598c425f4f3bd5f10c2301c673204a96192403b5195996ea935b7ed9f3c8fafea5fb5347cd7d1c0078b9ea33edbc3d373401b86ea21b3d086e844f0afc5fe042811ed9c38b6f4fdb0e0732492dacec6100624f1c68fb0ea68cbd312302d2f608edbd3de66a5f3354a3dcb9874524056cae64494c2d73584d19ab566831424903c48e33ab6b3b2f3ea395c5b3ea640e9cc0dcd97d2489c5a9902619a88fad6454071c9e78d493ba54929dfce481afbeef85ead7e0cb135f043e7a1314f7e39c095d9d24772ab2a7df32e9b8a9409f3f82f44793f65d9d41cb2a101f5af7e77afbaa3bf0e8c1d7748fecba5e61dfe54ee640db773065ec911979abac3c7df6fb96ccae635da84662110950cd50c9f266aa87d1904a6a512790a584ce821aa2ce2a5dad05d22ca36f7e81351b134ce6e8e86c61c7a8a016b29aa0ea20f1d1184ce048d384b1d171366f7fec2255f545649f3deb3ab87119bc43e022e0e40ceeb8113ee2e56a5092c27c859709bceb1a783180e40ec3331adf3ff73578f308fcb6154e52f8dcc4b5750a1c8b0a2302ed005f83f398a49ef1dcffc74a01d22fc37129fcedfe3822f65577811b0eb5eebbf155e0340007ffb41aae198387772f77370eb47d0753c61e99911ee4d4e5f217223146ac202179af67c12588cb50b52886163f23538760c9522555a536d64f6f6b2f89b558e3706ac0f9f812d0d25ea0565592384011700e4551675085381e632cd936efe6bb7e74da0bd5f13a7001bc1ef86e63d34a851f643096412cf084812f0abc68e29cc7e0f30a3701082c77f0402b24c08302cbc95bfaf5750738b4b8176e9ff8a270d76ad0093f833eb7f7bb2583d887c705aafb79ed3bf75477e0ce31f8ec14d47d177e6e13ded9a8eb72e0c1897227b6efce81b6ef60cad81bd32e20affe4f2daf3fe358d782f55450442157b7d3dc0e9114c8401cbdad1058c8ac50d78ced43dbc946661151a0b314b069434696183413fc007a7acbecdc91e2ac87a8820a6a05972a82434d426cabb3b60daed9af6ef54a185905af065e097c4d6103b99d902a6c066e06764dfdbd0edc35f026816b801f2b0c29d8c6e74f14de7c0d5c7528f6c5040edeaf708bc2f3a203c6e14fc98789370269e3f3bff74eaaebbe58970f57ff67e071cd1fa48dc0275ae015d7e6d79b32ae84a4055ea1f0b7c0c64679eb05fef3babd0c871f68fb0ea68cbd31eda1009ffd76d78f3ff8dae4621ca8685ea066a824f9012aa8b62052e2ebbfb27ce381141f8f934f58413232c2f2a50b28177cfa07fa7966671bb61631f8a0c12fd759706e3f77fcfc719ed918e25c865a011540898a06cf7374762ce29ce5affbd62527bcfa4d679e79e694fe673739fa995e1be4252c3b69a93901eb540d222238678104411b23598a31290f3f1372e32f062878118ac7d06005a73e5b765804a5bfdf47fc9862b5c4c66d75ceb828a2bba7c88ead8acb32d2945d0a47103a4420cb7c464746c888afd852df52608adf864d8e7ea655c53aeb2c4eec299b39561a236f0a60119918797bb603bb7ffd182a3ece18d459e238a11894b169449af884514452ad33b2d5b260a1e3252ff7d9b83e40b310cf33a87338e730be438c214d05ab8ecca53a30fc78d0d65e7ad19eead8a4c9be983e0179235e6747e1155e54f773317080432457c705015572c1715c7e3a5c72828fefd5b10eeab64654f0715a0397512a95197c3246c784732e2ed0bfcd31f04c8217e5867958308445a111de858a4554b159c6e0f066e2dad6fdf6aa376932c1b409c8b9f3093dcf5c12fa16a31eb943d022b8861dd23850c1a9a1a7e4f8d005861bde3e8b4faf322ceeb674cf8d587e7c89f9730b245b60e7fa84de65197e68d8f0584a1a1bd2d80142100a61981bef620015ac3538974aeafab166eb5ba7abad4d8e5ea64d40c6c6fd9346637b6cdd06aa92a1e2a18dde0311309a0fcd8a80282206e71cb81a0bdb0c272d89884a3ea349861fc59cbdd4e325e7097dc716191e76544621b6299e9fcf05b1ce925905e328b42ac54ec56fb5782d8e720b3a1adfcfad773cf192e96a6f93a3936913106bcd87fcd690bbd759c40842864cea365425b74514042fff1445c8488cc7933b0c2286a4ee41e44357caf0489db00863a32961e061e3049ca26231be8f903b0c934c095b035aba206a3174746472ca2983f4f4ddf1967d56ba4993dd983601597872f466f562be797f4dc6e228b739262102ce09bbd2bf29e4315acaa3fd1e5529e159052f03e7e89855e2917529467cd41a8c183c6bf0350027b8348f10f6038f30f25097e032435ab31cbf22a65c1aa57bd19af3a6abbd4d8e4ea645405ef70f1d6f054be479dad216f2d7dfa991ba3c4fb6d2b0cd9d60c4807af96c271c4a3e71ea276baa442ded38696c738ae727446288eb097e00ce298e0c27164f0c9e08461d592c5487a13ee231bcddd2161a8e5f2ad4b4aa4637cedf32f293e6685693fd665a04a41a27d776cf725c7b7520ef7e43c4038fa75cff330fab018a41739f3ab9b8b85de1eec6c09d4fc2da1d4ac180a841509cf319cf14bfa3c4b6c763daba02acb5785e9077450dd54d25f7a6e30c490d020c179cef31a70b2ce362656747eadf77f674b4b9c9d1c9940bc89b3edbf2d2f7bca36dc5fbfed0e8fcee84ee8e94d34f29f0eff7d4f8c71f83b51e82a2a24c0cf8820f62786c47cad71fb258f1312856c1aa601d789e21ea75acfbb5e06a4247b787cdbd8ff9250c187c6ceaf03c8b314ae6d738e1ec1021c1194bac430c566baf5cb7eed668aadbdde4e8646a05e46398a0ecbf2ef02aed817362454835e1d29705e00cdfbe3fe1835f551eda56020d308d512c11e1575b85cffcdc301a7b6806e2453807621da9cd831b579c5024ab5ad6de93d0dde2d3d72720165183a7823a87170ac6031b0b3da794b96f7d15214451d60f0cb17dec9997c7a5b89981b1c97e31a50be85cf67ac2348bbebef2541b16230f4c1ef2118a63fd46c3c890f2cc98e5ce75cad3a301f3bbca74963c6e7e48f8eafd8e4a6a72a171b060ee029ccd43516c6698d7e3e3bb98284da9550b6c7daac2acae90e1640769ea485303be238c2043297538969ee133325e61fdd68447b759ac0df584799eac5d37a7fecd7ff9fe4fa6b2ed4d8e4ea636166b5bcf9f8c76565a6ab5486d7b4d3c0410c4286de5009ba5649952f7e0ae75969f3cb493e58b0beca88458f5f04c6eaf6b6344cb660ed190ccd6d9b92164f3fd439c3cef78ba5b2d8f6ceee407b76ee7a9ad96f32e87a101472d565083670c7ddd016f3b3f268c3ac8140c194663896494b55b9efa73e02353daf626472553ab62d5a3bf1cb5867b7f1d4b24a6f1a08353481370d640260801b51a641ab0717b449c80cd0497095922083ece09ce099973a814b9efd7094faeefe07b77d7b8f3973e035beb74964376eef079e237969e79821f79e0a70461c2e878259fe1a175424d11e7b02803b5317d7cfbc3c1377efaf5a667bdc90b326502f2897f6b79af5f8f5b9358f5670f0877dc6708c4609da3e02ba72e0f3038d429696249aa0926b1580759ece112214d20893302af489a08592a6892cf306c9f13b2766b9575cfc40c57957aacf80543ef9c886210e28782b5f9d4db6a4538fbec003f4cf2901304358a1858fbf4b86c8b7732cabdef9eaab637397a99120159b16245f8d8d3ee4fdef6073e9bd65869e9f1f9f22d31dffc613e5ab5617b0bb73d5ca3bbcfc3178fb4e670a96034c2d69434b6a4714c960836150253c0556334b66471427da7b0e181317a3a4b0461465b5b4c7b177474062c5a18d0738ca35a7184be10f8ca451767bce2520fd3689e4a8aaa323aae7ce37ba32c99a7b8960d27fd70cde76624394393df5da6c406597ed5c0955e51163f990ef3853febe2db0f8ea2cb03eeba3be5c92d054686c6b8f0c52d2c9a2bdc755fc2a6272b1402a1548ac017508bef1528161d5e60983f37e5c4c50522df1194729ba27861c8d33b761045dd14bd12d54a4210191e7ab4441c56a9d7132ae386bebe9015cb4a0c8ed5f03c030a060f3ce1967b1c255f58dabb93a8adab2d36ebdea8aa77c84488719326bb71c8330adf7d3dc1d860d7ddadbd9533432fa4102484914f9c04d4923aa84f9639b22ca13e6a319ec1683ef5362c953011cced0b689f2544a1c1848e39ad3d1c3fff049c5aac288a41109e7c6633ddedb36829161a31c1cae0f8286bd6afc15a875a4702689a910f0e8011413c05f550214f699a0674b4f6d1e5ad4cfa4ae7f4be68e1bbf62b515c93df3f0eb90779e2a1f6d7cc5b9a9c29ce60c552a90ba3d5045c8a6afe80d6930cdf8488c668a258118a4538e9ac565a672564894355f3875e0d43d53162dcaed0467106c49065168c21c3c3a9021eed2db3104252577fd6318f4145500545704e3128ea845415114ba53a46d4b621ac67a77f923c69f394b1a784d3fb4af67c235ce0e03b022d7bd8fdd155f0d7fbbaf681b0a77aece39ac319cc7fa13cc55f86b20f5b98949df285ca7c8172f78b99c81e7fc83648db2cef7a3f0467852c756419d8cc2349419c4f3d8ef1bd109b5ad429a53638f1f48017bfb2444b574cbd66c952b0d6c729f87e4492e4d94d54854c0d992856a016c779d8bc3388e6b22d08f3e6ce035cee4d37e40639b970aa2ab83c301204e7c8237e6d957a36cc58f6c81fa96af950efc3c17223bcdcc1ad7b118e3f9f2c1c87810e6f5252efbde1c15bd98b70fcae734802f2d277f57cb8d4a6b3704ead75580b59aa58eb30e42122223e2e032f50969e6638ed3c8fb9f3853856ea7576bde5158751682bb6138411d57a1527e4a1240d7f8ab336b72b24ef0510076ae9eeec0405cf78795a3d913c2eab81bac92f2ac963c13426ae8fe9887b829f6ff897bf3d94fb70b0dc00af76708bec96a42e9f04c00756e599df0f37fb4c5ead7934dc9426b83e92386801695fb4a8a36b8e7b97f1539caa382738db78535b0378a456711666f72a679cefd1d5637189254e1c2211da78b3e7ee12c5aaa3a550222a9418af5599987aa80282c3594be879806b6ccb89827057bd448409915275f96f7936d83e4f14e14085381d93cc55d8993cf81a55ed3ed87b7130ac863748be60cfee71610e78f7aa034c70365d082cbf115ebeb7fdabf37d539abff748e2a06d90532eaebe2e8cdc022143d5a0367ff0a4b1ee87b54258702c3fa5406bd738d58a254b84b26fb099e04701cec68d8779424e854214d26e8b546a555045250f3d710654b531afc43dbb2f2f91c0985dc9a7a461c06b43f4a411f12b90e7e512c1a9c36529f564847a6970de3d5bbef42a2625289b4e6e806b14be24bb85fa68fe6a79fb2af8ca815c6fba7571853f067eb897dd7f3c95651d69ab521d540fb2e28d2bc2963679951fa63e2aa823d7956caece24a9d23d1fce7a4981a8a54ab52ae042d43afca880758a93869b7de26136603c0fdf8f08828838a9e7dbc923d8ad5ac4cb05229f3792df47953cdeab5c6e41209f17626457cfd19881925f4bf3040ff915048c305e1b2275837e2ddb7ea9aa4ee962317be2ab7996f22fef4138328155571fa070cc040aafbc019e9704fc4658065c7618aa34631c9480f416865b8380d71a5fb028a8c3596d088ab0fc4cc7b29385f1789c24533c31a8cdb5d55a3d468dc14263ba6de3415725f07c3ce3512c95495d865a8be7c053459d3654a5c643aff928d5c40ba7141626a95132e92fcfbf25221863c80565421573a42e612c1e2061e0c20776fe9fd2a1ddce7db31ade2f7996f2e7dc778544e10dab1a694c8f0474d2b2010d6df6797686c207e5b96ffc7b67a26e33c9410988093b5f546c91d039a7b5d4329ea4c4d6618db0e824a13ccb31365ec1f77d7c35f84ef0ac435449128b1f155097af4c98a7e811440dbee7137879d022dae86524f78238b588c92750792a788051302e4f27148561de0b19b36b7e080610b34b40ac738df07ac8552f45308cd546b4ae3be7452c5a3c45f7f579ac86ff04fc6f79be0a5137f0ba37e7694d8f18043eb3dbefb77d05da267e37be3f279e4d773be768e0a004242ab9f3bd2846101175791eab3863f10a47ef22a16022ca611bc606e0203026d7fd9de03044611967b56110009afb293ccfc3180f630250416c235529824130aa8de73e0f8b378d1e45d4a3542e010ed7108c09a1c8275449ae5a997cd87762d40c40c4a136662c7d12e3747f974a3818fe6ef70d0a55812bae86ef4d63b907c518fc9be68b644ed06a26ad94e5e5ab6bed5a7e40616b1ffcdb4cd67126382823bd588c172a0e0f43c18f4863cb8ab3437a162614b4801525b50eeb1242c033797286cc29adedada0826b18dc32913cce08a117e08987da381fc2955c10542d0d5bfd59035c1b46b7e499188b4121cf6a6274d7d8636eaf4c0861fe4f5e6aee63c9878b1d8a954a36c0b81b7a3370eda1dfd6fd62ccc0e557c31d877aa1fd75b81d88017c2da437e459e927af9df80185cffe55fe6afbc0e48b097cfe42c8563fef4a07c674b4e55038a81e24336e85783ee2e5b976bb7a0d1d73ab4452c23987cd94d426f9b06ceeb9c384054a2d6d448516acbac692078db4e702ea04cf0420906669ae7e1976bdf97d7c2c8ac3a2928f60e536b862c423f402a230c473e481290dfb63d7fdde658734e69b346c115541c591b984a1fa8669b54126a3f07753211cd38985eb15e2499b8ebb012e5f06970b1c3bb151f3e508ae3f0c559c760e4a40d4d913032ce2f270f6de25967254ce338da8626d864d32441d9167b062884aad94dbba7008d6658dc215d3188d728d512c41181d1f25f403fc86c0a880e73506035cfef69709775ac366f73d9fc00f72c1a121583231ccab18631a3dd66eaf1ec905d05267fbd8d3a8eafeac2c35157cf4c66757c83d22790bf40b7c7df236810fc9f397a9feda95b06306ab36631cb080ac3c7f65779c1a3c63d4da84729b327b6e88ba5cbfb7ce91a429596af114ea694a3dc9f2d8284b235eca3554a77cf40a6978421aead6ced1615a4b658c3414a486dd10fa01719ae4aa54636c6562e0d7f73ca2206c3802733b45c97b9989729ccb953400510f7106a716231eea521d898749f3259ca71d01dfe5eba85f3313e51d2ce6f9c6faa5c025bb6d3bea8cf3090ed806c98a9c31385461c1122343d584652704f822b96aa54a9659e224c5658ec409499c804911df278c5a5167f3a04369d826341c800d756aa43e462dadd3d5de05da30c41bcebd200ca92731c5b0c4ae2c8d2a602020a010449373373eabbf910fef22e0dcc4de899c2a063116424fb63c3dc253cf6c389669507d14ee9649ab53356ae6015fbe01bc6b0ec149399dfaf855f0e06ab88bc6b2737be0ce55f0aba92aef77de5158afd6978f0c59b24c29969539b30dce66b9f3cf2af524c5a68ec8e4a34e2818a7d8da38cea564d6e609e1049c2a6ed228536653b60ff4d3d1d64e392aef7ad85d238f6f6ba944a5566d249c6b04943416e571ce611b3157bb024b1a1faaaef149e3f886aaa579f345603c76ecd896b1fed1fe69711606f98a477b123c23f0c51bf6b296fa11c25e9761d3295ea2ed48e38005c4446154191006b6c7f4ce178c389ccb279e67a912d7337c113cc9a3698dc99f00b19634a992647143edd15df602000ac39551c66aa32cee5d94ab5e133bd4204e682fb7315e1b6d6cd567876c156a498da1f19dbb7a865d976d046da9cb05e359e762631aaea7103846c703d6dd532532665a264f5d09e3165ea9f0b33ddd56e00baba738ec7eaae8857f270f677f0e0a9bfbe05b87a14a33c6010b8817d5b2cc73c431b4771610930fc75ab524598a51884c2e306ec26fe109be07268d737b4227a2741b82d21086244d983f7b3e918972154a00971bd72a4a312ca3a2c4691d3106cdf532c0b261db26529b61d43c3bb43bd13c479e39420599c8ece8393c5fc00b506b78fc51c3f00665fee2f9a3877a53f7c65ba012c0ab149e977228ef0bb9fe06984e5fcc417121640a9fdf7dfbc4d0eee1a8d34c71c002d2d2eeb99659862d4f6698d46b78b36d231cc4ee326a1a6603228247de9344064c63046b424044722720088117e62123bb5421b7eb5af9d459a11895a9c6b5679de5021bfbb7d03f3ef49cb015cd2df3866030c9839ed7c5f73d4c600843cb602de0fe1fd63152e4d88573a7e9dbe32d00000bcc494441547534e64aa8257005f083ddf7e5ca269f5b7d64868fff1ff2054d27a8a7f9b6a39a0336d20b7e8bd7d2a38c3e6df8d59d310b977aa499c5030295dca6e0d9094b9ec99d816a0d6a1c9e28561bbd8b4c78b60194384b19ab8e532e1477d91822f9f4dc8682447bb185d1ea18ede5368c783cf9cc0636efd89a5fcbe924d54c1ab30e1bbf445027887118dfc3f882f11cea45dc7b97e39987448f3faf437cd87ec877f505783bd46f85d7eeccc3dd2fdfc3219f5d0dfe2af8f4fe5cef4066e61dac117c0d0cb0dbbc95e96026da72201c700f22d5d2af6c4d695d627547e6f1f39f2b81e46a501804b9ef5a27c68e1471132a7d3e842bea1a9e6c451bfb720dc8a1e2d836b2a3f1c2172659280da7a0a1542853a9554835e3a9ed1bd8dcbf15682c2fdd086ad45d818c0dc1927cc8179f5c300230018485907bee526eff7f558a5dbecc5f300f11597bf0b773ff7915c401bc5ee196bd1cf2bf6ec897666e7218396001b9ed1bb7fd746073429a64a292b0eed1945f3c982f5b10fad12ee1c8bd75d250b31afe0e75b967dd69deabecf285e49a90a830363eca8eb19dcf1ad413a3550da747144618e3f1f0136bd8b06d13561cd6e5091bf2b845d965e74cf84a54c0f810f8063f14fc50080b01bff94dcad6a70dbde7962876477497baefdb47d3a79c2b21198737901bc1cf43e07fae860fcf649d9a3c9783f2a45777f87757863c86072d78ca03bf51ee79c061c252ee197702eaed7aae019c0a696a71d69239251ff852d4e63d893850073e1e4f6dd940625314c10938c9d5318b65fdd34f3230ba93f16a25b7632c886d8c5669c3ff311156021823f87efe6742f043a1d8e6b3668de5d1b51e9494b01ca8bfb3c0894b97ffc7d4dddafde35a487b738ffa37f672c8ffb8013e3a93756af22c07a5c39dfcb2633fbee094a18f6e4baad830256c2d138611bdbd19e7ac00a3c3443e9886136f222257d49188471a14513cc4e46f7b317908ae27b90fc38830a7bb8fc5f31637424b94fe9dfd6cdcba890cc57379b76379d613afd6e58e40267c2120a6716d4ff17c4142831fc0bdf739366d1442afc8af6e49a80e9639e9c41338ffb4b3e67dea2faedbbad78637f9bde3a0a2799774b4fdf32fefddfae1ce15ceaf4ac81c35148b305229f1d307ebf475449c7eac205e8c1af07c0f750eabe0f051df34742ab36b2818f2f54054044f95fee11d94ca65927accb6c16d54eb753ce32313467e635c54a53143d0837c84aa11a16b0c9e188cef0842438a4701c799b37c5a4f107e54f7f8c5ed09d5c18052a730aba5ed8ee33afa86a7f0de36390a382801191969eb37a977e393f7c77fd8b9421829406b7788170a04058e5dacac3c7914d580b15a406d5c198d532a89e28230578d5411b5f89e698c54e57152cee4eb7c24aecefa2deb50ab38a304611ef6ae130b7f9a5cac3c1a6a95511087c1e0bcbc37329e22c667cb0e61e3a0725c6b4acf9290579e14b26520e5765fe8381ebcb1367abb17fcf0da6bafad4ee1bd6d721470d0c364cbcf29bd6a6b2df9c63894c2b2a1634181a52b3b59b000fee48a944269182375caf884048cda80a7471306528f6acd319ee6f6873511e22c6a7cf210740fcf6438c0731e9997e13b1f45f19c8f6b04308ac998100f03601a8286522aa474157d66153bf8d73b8618ae052c6a33bcf5dc90eea2e5db0f1baeff8123ed2fe8d61ff932af67c18eaf7ceefa0b4f5fbaf49129b9ab4d8e1a0e691cf9b82b82ef2efeab9ecbb5ada2838f5bf12a6df89b85979f5ee4d5e724144a83a82688da3cfec904a8164020c347d5518f1d99e6cb378fc542a5268ca7605343ac962c03b54a2a19228689d9856a3c7cc9f00d8817b075c032349671ce716d14c4315081c7fb6366b518ce9e1770ea5c786ac070d31d865b7ea4543640a5df68b9d826af3cefe57ffea54f7dfa48c841d5e408e39004e425c7b5f48cbd3ddb38b6d31407eeb23af78240e69f18b1fdfe12f3fa2c179c5de7b4930c9d5d8e76e3a3804786c5e54b2190270a7a748be1d78fc5a44698d55ea0bdc5512e841402c5333e681563023c054b9eab177558977be9450db149515720d498b583111b7654f8a3b35ae86bb78cd602ee883cee1a16b66c8ed8f2b53a4377661a7a6559ba68d9776efff2775e3345f7b3c951c621e5e6bd73fdf88e650f142e8d5ac33b17be3a910f7c38a1832a3f6a8db9fd07251edb00d1cd55da3b7c7a7a1c2dc514cfcfc3dcc350098c21b6ca53db52e6cf6de7dc132c1dad8e391d86ce76f08d435c1508c06418f5a1e151379207a864e2309a626c19631210c3e28e945bc7e7716cf98d7ce9a737f2b53bb6533aa797cd3f1d67e34f8708d20281b4c9b2150b87579e72c63b6ee73b53712f9b1c851cf21a85836bb3cd675e56f4c606edf9e79debebf64dbefce2b688c1e1613cf1c0f749928c81aa6534f51927c27a10073e550da8644a5850c62b31f73faadc7e57ca9d6b12eefeb5f2e86648639ff97305c19247e0d258fc335fe1d6c7c393887add31386cd8d4dfcafa81a59cb8e80d3c5169a7e68ee39e3b36b2f16743943a23746b9b8afab2f8f4aeec92f32e7ccfa7fef8e3475daa9a2653c7942ce279dcf6faddcbaf285cf0d07a5d34a74bf48e7b47a5361a30ba2e26ae5b8252845f3624758b6712c6460d434f43b91dc4836ad5327f96c7c5e744f4cd8d98dd65e99d6398d56508cb427b9b128a20be87ef4c3e82a54aff88cf4fef57be7b7bcccd77a6acdbb4081bbc94057de7214184532190168e5d742cbfbeeb09b6fd2a533c27275cd6c749c72c7bebe73ef8d9af4e45fb9b1cbd4c8980ac1fc2feea7bc917cf7ba59b5f0f0b2b4f59dac2c6a7a0f2b4a3bac951dd9e11f51578ff9f1658b43c220c0bacfb3944c7799cd817f38ad3cb5c7e9661a4ee786a23cced68e547b7f87cfbcb830cf4079c73ea6cda3b8b54aa15547d3c63c1b5f29e4f0e73c75de099c55c7ac1eb79d99917d2d73b171d2b30b45119d916820d59b9722ecb179cc8ed8fdc23c79e3b8733e69dfaaa2f7cf00b7b0cef68d26432531e0df9817fe07d4f57f9ef4375af2b1e373ad4ef33381c4bfba2883ff958c86db7467ceb2fc6319d1e1d67385e726e9915c739babb3d3c27208e754f59d66f334451111b0b83fd556ae319c7974b5cf4aa0ece3ddd63dd4017377d7d2e671eb79cc5a7ce214d63ca2d86ed9b2cf166210c224a059fd685916e1f7e5abef0ddd52c38bf75cb258b5ef35f3e74d25fdc38d5ed6e7274322de1c29ffc76f1cc519b7da22efec56389f0f03a4b1809e75d64305e817fff8788a71e8ef15a6392d0236c3554461dec74742d0d78d15b8a0c3d92f1db9b53ca0521f5026a5ae143efbf8aa5c72fa2b31cb073a09df1b58e948cd2024be79c88729bcff6a762fc5a1b7d0b3ad934b249d7acbf4de26433717bf1733d279dfcd94f1cfbb975d3d1e6264727d3164f7fdb6d14b645e187a4533ed9d2e671dbfdb1fe7683ca585589e390c7ee32bce8bff4b17c6c18a7019bb7a63cfccb84adfd09275e328bf66e9f35dfabe3b6c34035e3bc9567f0fe3f7b0dc536c12519839b12aa836d14cb505c00e5502996225a3b5a904ac6dd0fdca48be66d96d4768c3cf6b0bbdcacfdd93dd75dc7b44ca76d72f432ed134e1efe65e7c2874df56fe61d135c5aad7a731ede1af3f82647dd795a534f22e771dab222272e028a45c495786ced1837fdb08249a0d85660f9dc255c7ece6b99eb1598d3e1d1dd6e18b145ee5a5fc38b84ae6e8fb0a0ea33208f6d7f880d5b1ee6f4635b9f191a58f6e3dffef68befbcee4a49a6bb9d4d8e4e662cc5ca4db7474b0a3de675e582f76a09e5ec314dcb49220c8e873cf974a08f3c9e48a927a0ada4ac98df467d2426dbd9c1890b5fcc45279f4a47a188234681ccd65019a74245ebb243121d64cbc8009b0620898bb776b475dd62775cfabd2bcfbc76d34cb5afc9d1c98ce720bae96e8a7e476ba9cde84bd3d6e87545dfbd2e0ab5bdd5b4b2bdeaf14c2dc3fa119aa47495033aa28852c1a31409ea7c104b6232520bb5c423493b86778e9aef18bbe45bd4daef1d2db6ef7cfb92ebea2f5c93264d5e98232249d70dbfb8fc98a075e0128fd125516b7bd7b0967b1edf522925b1165b5b537fc19c2c2c17da7d9365632985752ef31e957acf964efbd25f5c7afcfb9e3cdcf56fd2a44993264d9a3469d2a44993264d9a3469d2a44993264d9a1c91fc7fe17f09029470acbf0000000049454e44ae426082";
#else
 QString logo=
            "89504e470d0a1a0a0000000d49484452000000c800000060080600000058173d1d0000000473424954080808087c086488000000097048597300000ec400000ec401952b0e1b0000200049444154789ced9d797c55c5ddffdfdf39e76e594880802ceed6aaad5dacadf5a9adb19a04dcbb415dea56ab800a512a46457b898a084895b084a04fb5fb53b03fb5ee04b75a15ad58f7a7ae8f5b41658740729733dfdf1fe7dc8d4502242c7a3faf17243967e6cc9cb9f3b933df75842200d8ebb6ea688793ec11493965293103d5e12ba007007b1be82faa5528e51809012a90109155827e82d5ff60e42d51792dadf6151c677924125af3ced0f9ab76f47b15b16d901ddd811d8a99b57dc10c12c77edf15dd5f604f57ec0023b80238a218515c51424631a85f2f1835510151441524f81d50cb2a317c80eabb22bac05abdffbdb39efed78e7acd22b61e9f2f822842d3e030d1f41978fc12630e0cae83a2c18cdf6475234ac4b1c41c4bd4f17032d7b17e35f149e373455555318e880282aeb1e8cda9b0bdf9c35717aca411dbddaf5bc4b6e3f34190969a3d5133183819380e20580cfcff256f1c142df83bef065a78d9314ad858a2c623e228116371445140d47f9431822a6a4405154414451e53f40ed749dff7faa9cfbcdb0d6f5c4417e1b34d90e6c15f42bdc98856a3a604b3a9f7f5bfe337fa67b0abcafe6d553122d9eb412109d69e52d7a33294266caccfb5a0a8f18ba964c9a78ae81a545ad77ac9618bcefae7b22e78e322ba189f3d824caf2ec30d7d1b953822df4375092a1f217c82f00af01a1e6fe29af749b57fcc458fb76df439b79e544e6a5d3f8cb317d6ee077c193818952a44ab40fa006e1e910a5816762c95a11451c71232367b23b38b137f89f17f02aa729ba24d6ffeeca917ba69648ad80a7cb608d25c7729e8b920ef03f700cf62ec47105dcab07bd775491b2d279490b27d08d9be78f6100cc7839e0062800d88628c123596ca709a32279ddbd305c4405123be9c8fe85ad027adb117bf79ea33ffdb25fd2d629bb0eb13245eed3220fa55d4bb02e46e86b7fe6187f46366cd20c45c08fa3d90528410009a5171f9abca6e9124116331c6d70c1008f528082862c5ff5b66244289b1ef0c5d585415ef40ecda0469393484ad1a8812e182875edfd1dd01e0b6ea4a3aa2d56007034311e905f8b28b3ff58938968a508a8a503a2bd467898255f15718417509c83442ab26be36f4b5e48e7aa5cf33766d82ecccd0606c67d50c01b91991fefef56045096c26fda2492a4229fcab026291408d260aaaaa38f289889ef8ea294fff73c7bdd0e713bb0c41f6baad3ada01fd44b50f22bd45b597880c54b1fd10aa0c54aa4a196889888444ada8112b2a49d0b522b489b252b14b0cce27c062c42ef7ac2c17bc6525693e79fd174fade9b6176819743c9e3d0f916310ca80ac862ce27854455294bbe93ce15d324647358eff39a97a2d8837fed5539ffba0dbfa594401766a82f4bffd7b758ad488528de86e6a2526a22522c454c41132d26d76f742d6a82de4a957832fedc02208c17dc543e8c06a3b46d689ca1ac5fe4b847f889547ffefac7fbcd1a52f148f1b063e3380b4578fc8a53e03722b4ad828fda209cadd341a6cb954c148c636631158a1962b5e397d414b97f6ad888d62e7208822ccaeee1d22d4bfd47827184787a01c923341f85fa046d4df85e4d91d0c8a88e65e24d02115d81fc89045b305440a8a676d14820d266720352b8f08dca39e3c4238f571cc5db3a24be4815bea7a91d669889e00d20300eb33bd4728cd6ed1046109c86f329d0de41310c5cef75cf7dcd7863ef9fe36f7a5884d62c712a4a5a6022ba7201c8bca5710dd37d0e56c1ac16c36f8ae1f46fc9f215142c612319690c999c9337687cd12848cb748401002a35e1ef3d4b248843751f9178ef7e0dba73ffdd0368f4173cdfe20e722d2000a5614418c2815a134fda2095ca3a8e6c9279a11f8f56385292f9fba60f236f7a3888d62fb1344119a6bbf8c700322c777cd330b2de12210733c4a5d8fa8c9b93c19d9028208d932e495415525b33c298a63eff090a69e6d1dcf2c3c7f611af9547a6f1aff3db80f493b17e548dfe21f90c5a8ec11eba022ec21d8401d9c7519532388b5fa28e97527be74e64b6bb7aaed223689ed4790e6c15f02ef64444e030eeed6b6f2092310311e51a3441c2feb2fe56f9f02ab36d9a29b2748f0bb5a544cb025f36589a5601ec4d26a42e9bf6fb58fd5ac9a63b0d28091dafc15a52c94a64f24490f37676c44c0581483a0ba52d14b5e3cf599dbb7aadd22368aee27c8ecda7d48d382e1bf5029f367d7469c0421e728b84987c1ad40fef77920ab94381e15e1341163031b048110bf0504c97ab85b9f67c1b2a2aa9e60d62afa9aa74c78fb8ca7feb6c57d9e33c461c58affc2f2ff10d3c7974d1009b65d7b96b4071d118cdf17bf0b6a558d3cfa82d9a38ea173bd2d6eb7880dd03d048957bb0c081f8aa70d18f3c3ec7555504921ba1295360c6bb174203609268da8efe1a7e220ea02219028680942292ae508a5feb3f27abf4584ca79e5bac6d2239426e6584241ec07e89611446cb6f1c0201ec83a123465d7aa325591b96d6b136f2c1eb6b0f32e2ff1b8a1dfd33701a723f4ce10c588b247490715a1344672fdc55a152322d8368bfcf485539eb9bfd36d15b151743d415a6abe8b351310fd4e204b83c893a08f6165018e2c02598549b691f0d6b2840e1a1f4f6ff45973863874ac8c92724a486a29c62bc7732a410f020e43f408440e2c604ba7c992238a042ad672374d452895b99b551dc396122468c16a60c350ac95a4a06f227227e9d4cdffde12efdde9755fc0d57120a7e76fbb4a5c8f3d621d94ba9eaf00568b202a0651b569c5dcdf1e353f7bfd07dd68dff98ca3eb0832ad66002eb3c09c88ea2a8c3c02cc66f8bc07bbac8d4d6166cd208c0cc58ff528034ad6731edc703bb73ef2dcda7b8552f408a730e47bdd1692a5530451f27dae7c3a0aa228aafc45b057a77af67cffade31e4c74ee3d077d0fa37f0606e613a55f2c41df4812477c858448a0e9f2a34f9659ccb12ffcf4e9e7b65a81f039c6b61364fa71fd30c9eb30e6fb280f60f46f84937fe79cc73bbaa07f5b8e96ba0351fd2670242a8310f6ccdeebccea9289aa15a5d4f52877d394baf9db79d93a82e435ecab9f33735517a8983bd5737effef339e58bcd9f79b5e5d4628721156c7225296d976451d8f7ed124bd23c9ac7a5b058cda8c3de796a4176e78f9f47facd86c1b4564b16d0499597b0a700d46aea657c51d3b9560a808e3e2c2c00507e1795721e614728eb59bad4c36fad6b7b1f48d268939feeb894a46aedf0682a0824aa0955044d2a073053bfa95d39ef978b35d9c3ab80f117b2770045635d06a495928cd7e65edb8e22b1b8c5a3fc0cb6f794d5aec892f0c7deef14e8ee2e71e5b479099b50763f47894b58c983fbd8bfbd43d8857bbf40b9f05fc10e4db0855402756959cac1271fc68c132d70bbc7065eb0992299fe1ac6f1e07b1287aa7815b3c27fac46b433711d0954173dd99885e0df2859ced041918eba02a92242c36b3c70c946d8a22bf497a5cfb6231dc77b3d87282b4d4fc026b5ec2845fe9b220a4ed8939431c962deb87b875a85e83c8ee40e789227eb693ca508adee194bf1464b45e6c334154c48aaa00d613e44355f9ef574e7ffada4f7da75b4f2a27d95e8fc8b50864b65d2163d9a3a49daa480a4b4011b272d5328449cf0e7d76d2d60ce3e7059d27c8cce37be224bfc5b0d679ddd89fed0f5fc0bf063810951e593b4d27c86244e91b4b50ea78b8b9d059b2ae535b4a10023b0c82c9f2ce5f5614fb2be3c8ed2fbeba60f12633a2cc3ee68b58e77fc07e0d8cc9f4b3229c62af920e228e0de474df95c6df2bda051e9cfddc90e7de280af11b62f3045184dbab2b488694619fe14468cd755f43f547089720525e6868d918725baf50604fe91d49f936908cffd7b61024d74c602404abba1ae4618337ebc5539fddf81755bcdaa57fe4c728bf461880fa6644312abd2229f68cf944090ca4811ca46990b969ece50b872edc359d1f6fa9ebc579f39677f5633b4790cfdb37cbacba06d03810010c88bf6df9b4ac28810770bfa8ef0e9299fd264867b2d5040910b8befb1ec7bed6f81dabe6ec97fffdd4931b5d517c9fb7168c9c976d35500bef5ed241ff58c227b328c68aaaf82e66169d948ed8c685276d8141734741119a077d11478f66d8bce6ee68a2fb5d4d76554c393c4659f979583903e19b9b5d5020ab228e38969ee114156e1ac728da5504c9d4d35c0022eabdac6a6e4b4bfa8e8d0652cdac3b14d15f219c9425ba2011c7d22f96a04f38e9a72852203032a2f663456620eeef9efec9d3ef6dcdf0753b66d41c8f9133501672416bb779331709b2394c1d1c21a48721fa5b847d366f78cc6cb11447a05f34e187d44281ebfd361184acff1522d677d0813615b98f8ec88817cf797c6541e578b5cb80d821a877272a03b3b58d8a1118184bb07bac3d705791ac9bbfa86db7c25d292ba3ff39f49f1f75d1886e1b9a6b4e45642288c1ca31dd9d8ba048902d4173dd99a09722f2156033db2ec87815c75c8fbe9124258ee7dbb6f3b55e5b4b1072e54182d0dc4028b2f207c4ce081bf3e282a10bdad77b87b1a81d8e31bef62e4826e118658f92767a85d3bebd473312960a0614fdabaa3343c579f5e91f3ffdc9368de396a0e58412bcc4be0827029720d207b5f3d0c8295c705fb71b3d8b04d952dc561d25113a02e47a90c3804fd77a655cef45893996be9124e5a1343688ebe83282a81f9aab820449b6536ae5352bfafb174e79664ac1c35aaaabb0915380c908d1fcd444aeb194873c064413f40efb5679bfc1a083d6be0bbcacf0fb7f0c5938775b87739398597338c69c81e57b88ee8750824a07e8190c6ffdebf6928b8b04d916ccacfb31e80c842a449c4fdf7ee5b45e256e9a81b1042151ff73f6f74adb4e10b3811d460d2a88dab49571366c5a0ef0765f3637e3f1e06bbca6839e8e4a69c0d8c0791e091bcbee251df48d267133dece8a4a10fa0c60551f13953938ee03b1882e77d35e7af162529b0d1e53e4d0d987bafbf60cb96bca6ce8ff56ba659e95a3de6a2b3919e52788e48640d503f907a57a3c67b6764d50d8f4ea3212098f5faeb7c2ae872241b615738638ac58f9333cbd0463beb6f90a39357045606c2c75bd40ed9ab3cc77094100a3fe8c0f26f45a541ef00c77fc6be8337fc93ebca5664fac9c8e703ecade81ca2cbbaa88283d42697a843c2add54264420f0f2caf545adac10b1ef212c426509a2cbc5cf3fdca1a29e2b6a4425ea182d77a0675bdaa9faa423dc7f6532b4e71acfedbb3aed66fd7008c603d12750bd8111f3bbc4757fc81c9cfb561e337a5d2af418173eb8d9344a45827425661ef3638cf90d488f4f3736e65613801ee1140363095cd1ecbcec328290e74d6c7c4f6254ac281d6a7462e9dad8a4c7338ea5f1b8a1df532703cd88ece6d723e7c016d04180a8b1f48ea4e8194ed1c34d6bd4b1bea74c5e028da0b866126d786a58997459960ac92789306bd34eaea304a4f40b0bca328c9ec6f9adad5db59d3af1ee43befa465becaed7d7949cc7f0f90f77a64e91205d8d3943c22c5d3916e12c44f602321ffc66b45ed03792a0329ccac6d16703a1e802829017879fd92665da40ff68d5fc251992675eca08e0be87c159a81c8eb04f5e770b15de99c92dfe51102147fd83878282568594153aac011bf42cb34204069d3cf55e0a780af4f78c68fdef2d1ffc8d63f0dd877cd5f3f4e277d6959df3f6caf2f3b9e8815b3a5bb74890eec26dd59524226783de147cf89feebe12cc19c72815217f45c94cb2cce4ee32820475b3118a062cd6132b9f6058603d3be1b9539ef3b71f2dd555d8f001c0b90867f991f8d90682772a5c113f15c28663a1ac046ec4d13fd1ee2ca2be93f1319bc177effd6ecff254dbadc670cc3f5754547cd21139764be3938a04e96e4c393c4649f9adc0718854029dd67a0d8826a80ca5718d0dc48242a7c86d254846cd6c4cb6e98c508f453f1695c98e67ef0a39ed1f3f3ef435dfabb8a5e6bb58198ac8f7819e6810a09649d6bdb1f7c92d5d1eb00e68436425300f91df31eca1e7b76264378a435b0e0df5e9c39e6ab4de888e4c788667975724da92ce302e68fded963eaf4890ed85e6c15f42ec2f50bd242b046f6edb85efe75519f6f363e54eaf92acd6b54b09129457f5a928a258d5b4887945e03915fbb7053f79ee9e6c375baaab483b0390505fb05508fd41aa8032d0107e22afb5c07294c5185d82279f60ddff70d1fd5d6b7854a4e6aeaf9f6bd4fc548c1e1132c43eea08f3f2caf27422e5fc800b1fba6f6b1e5b24c8f6c64dd595c422ff035a4b7e58f066057aa55f2c49df48d22f18845a750741b2e5735dc86eac14c5a83c64e1bf55cdbd7bb04772ee90b91937e1ed074586cc1d62569837f7c531973ba23f17cd9a5e797b6d4cde5c55b206c3b18c687d726b9b2912644761e6a0ef81370a313f66b3a98e722b8a6b2c5581f628161cf3a6c1bd6e230814384be6e9b550f55e14e159b1f20222af7be2bed55dfe5b87fdfeb01e25e5de41ae7a5f41cde122fa7d23ec9bb9ef28dae639f2bf6bca58d21e7a17474e67d8bca7b6a5cd22417624e60c7158bafa0b88f71bc47c67b3e5f3bea38d2815e1347bc4da7145b18a7f6028dd4890827a7e86493fdba3643453092061908462df52e159f59ce71db12f2fd5d237371b1d9987eadbaaa34e8fb62f61bc4344e570d41c25502546232a1a31605425d8128a1aa3b2685d545f5d53269ee5255caf8ef31ed97ce8f2665024c8ce8219b57518c621f22dc0ddbcd62ba76dad8a24e91d4e52e2daac1d229335b2db0892fddd66fb919f513f63ef934cbad75c77d7004b45a50da31d2869458c412308a5a25461b467a69d0cf904c965b3945c827103b47b0e6fb595f09f755140e7134d9ed85549438a04d99930674898652bbf85c83528472374c221d2ff21a2943896aa882fa72860c9ac26db9f20d97ac1f3fcc92d9998f8f556b9dc03fd60977cc366615f8353b8100107786b4d89beb73646528da07a191f7f670a8d8d5d76067d91203b2b66d47d07430b70c02655a8eb23204b26f3626528852b4ae6c0d0ec192aec0082e465ab5cff3905e5a5b0fdf5b67518f189bf26edf2c28a724d788e20b423fa6386b53ed0a971da021409b2b36356dd60e05ce0270081544e2e95cffac809f4bed1314dcf508acab04f16cb7af6945d8120c156cd004b92613e581bd5258970663dbc0f2b23b8705eb79cba5524c8ae82e9c7ed85936e4638362041a7d315114cae3ed12403a209dff00805daafa0c6ce4510cdf57d55dae5b55565ac493bb9f732f63c86cdbfb5b343b83528126457c3ccda83318c41a905e91fccb24e678c44a12c9ca677384599eb11733cff801ef227eb8e25887fe29c2f7caf4c86f8a03dc2aa8e902f8bf92ef78f62b49e61f35fded2e1db521409b2abe296a37723ed9e843001e80d6c1951f06515d7f8aeec7d22492a43e9ac5e803c4749e86682e4ad1422ca924498f7d7455993724959937b2fd57588398da43cd855fe5a9bc3d613a4b9a61e31872376f80e4907346bd070b0df21e19cb7bd066ba7454bdd69285780ee0d5216902090cc37f719e76fc5fcc4ddbd2229cadc34ae049eb9a28564215f8ed91282e49e93795cda0a5685759ec3c789081fb587b12a59937d50b20de52eccb29f336c616a6b87696be06e754d91cb81752475eb9fb12d507b0df009e995eb874e7cfe306cde9f803fd132e81ba83d0ed17351b377e7b65f9939e813657932c4f264080422c6120ece7d2c713d4a1c4bccf188b936906302ad9866b848c14fc81d7b27a2782ab4a71dd67a0e6bd30e6d69870ecfa1dd33a46dfec7a8816a5bc0ea74c46b66c423af75d5706d09369cdcf1b861bfa762ace5eb08bd48f3017d2b5f63c8dc14f9fe364a0f44df271cfbd490c56e83481f549fa52cb5f1b3453e8ff0bd629f07aea3a5a61a6bae038e40d5664358374918d9603f91b0868435ac517c1b797e69816870a45d3838d6ce35996029c12a782a24ad90b48676cf216d37d6ac16f629230e59fe86e79dcbc8873b7f8e4a37a0b0c7cd75b50817a3d6827ce00faa1e027218ca1f30894b18f6f852bf6c6d3bf00f3efacea0ae34cc740a8ad05267516e65f8bcf3d9de8e72db8ae69afd11e9cff0d6bf777f5b470f449c1ff88647f92ed0377b6fab8fbacbdb966d69bdfcbdda0646507d11957b30a9db19f6e8db5bd1409723b782ccaabd10f42aac398aaa9e6f658f329833c461c5aa6f034f42a805f807b75547e94010566d777200ccaef68552d18f773972f8988aea2aa0fb0932e291ff0033983364169f7c12c3840e061981c81905e4d822b26cb8da74ba5ea62d5f3acaecd1fe84d11b20fa36c377ae84e83e4166d69d004cc7e3682e5c2f11974f94a798557b351a2cd3ab63e5843d41650725133315c12f9b3f70666784984341276ed736fdcfb10d5810fc3b8b96da6fa372327024a2bb017d417a6c50776b561a45b3e9e473c4005889b018f837f01786cffbcb46ebef2470993ab807e24d02eee6c2d647375952e449acfaab4a28558e3806dd411354dc72ac026cb3b7e60e425f2c2fece84e30acf519e0192008ad75fa22a63f56be81e19b285f45e4c08d9223e77c98f747de450996196511a2cfa3b200b10bb0ce87246531f50faeeef6f7eb02b8b8f6608483f09c533eb5e4b07939f23852eaebe264fb65d8cb874769e080b07487b4bf2d68a9d9334835bd63c66e53f065cba5c06b4061c68fdfd596d26e0ec0d37d300c00ad42a40288821a9434e83a9015288b71e47d52c937b9e8d1f776c09b74295c1cce04e0c2075fea742d8f528c0a9e2cd9e09e1f837d0eb02fa26fd1abe72d597966fa71fd70d3d588ee4687d35260bf6819bc1f9ef70d46b4deb151b9a2b9e654c47c1dd565401baa16c7166a38a60eee41c4fe1c651fe0034ce276863dbed43fdb24558dd21709ff8161f7aef30fd2597906429a61ad7fccb6d952f3152c4350798c0b5a1fd9a01f336a0ec2911f63a517a22f31a2f5f6ecbddfd596b2d6d480b73769fd03231f5e963d53506525c31f9a05f26db01e621ce60c09b3a2dddde90f22f293b56534649f2b18ac5663e58d2dab2625600c8e169ec770537525a5e5cfe19f349b46b999a5abde63eae008336b8fc6494e46b501ab9388da430bea5a3b0c238d8c8bafa7593b7a20b36a17a3a68eb4fc0ee421e068c0236d72499a67d61c4ed87b0bd1d781df0083d1f0874c3ba63792b809cbf9a8b6904a0e60eae008cb56dd0f721bc8ef69a93d0180964135a87909e46a8c3c4cf3d1030bfa32abeed7182ec67217469703b731abc64f4f735b759475fc11b5c3812938a6160027b210ab13503b8de69ac7f1180f78c06d2c5bf92e36f924f178d196b393c245d817f4952daa65280505cfcb4dd02987c788455e07fd05235afdc0fee641cf63f42f84bdc98c681d053cc2acba73506ef1b768f9b05f43294c463cf3f89e907a0a4b9c0be6cdce5e6fae5d8870228ef5f7b12d2794a0c93b107e9add0aceaabd06e461a2ea72dec367d3525381ca4ac2a603cf3b1b2481ca9160ff029ccdd4c1f350ef46447e88c87e2837823318f0093073d019a09730627ea6dfafd05c7b10989f03e706013a3f6076edd7b166306a57d05c773be8bf809f2352c3f0d6469aeba6002763967d3b6715fe6c1ddaf5598241711029dfa25a6a622010727207d4979437a0aa0c6fcd65bd18f1d01c94ff033d9adfd596fa75350662410ac32f558e01292488495e03bc58400e1f478278f4ece93fc3acf385762fd82e0248e4393cf6cd865d262905014f5d84cb30e153b8e0a12710790ca584a8771222931836ef2e441f064d83e6bed9c5b6a3404bddf7f3fafc218a32e5f058f69a4769605ade17d5dd18d17a0a235a9f64f8bc46ff39ba1fca3a526591ce0d76113b122e860f81fdb7ac9a2df175e19ebf77f6ed227508f337282a3c009c4a7be09222f4c2e2e1d81c19660cfe2ae201ac635ca3d248600c948bc0feb4e079cd470f44643f80ac6c73de231fd35cf728c2d934d7ee0e4e3d3d636f31f4de5c8c804b29d0063a0e955f31fcde7528c22c5c84c5a8fc10373a0cf027b9a8012727488f68bd838c8ae696ba5e24dc3026d51f858204c84a04832272389e8ede603c943e081d78b1edea5354c4d6c1a0f2180a340f3aaad3b5c4440148c59200a42325c0de20ffd9a0acea6a54da714a7cdd8d6537c47ac42427608bbd00781091dc39ebcd753ff275e9ce7a8130a123502d3c2006a077c5b9a88e063d12f15e65d9cabb993d78f75c3f4c8f40dbf24d46ccfb63aea22d031988ea1bfce26ffe8a28f4026330b2a8a08d5935c730abe6cf787a2e6eaa06952f82143a61184a037b51828eb677371c3be9814892e5ed3bcf99f2456c1206952007aaded4e95aeaf9db0327e14ffa0e4f405dd0fe1b9415d90b917fd3239299480310f1b269ec67d6f645b40d218ce2e4eae917c1a610c9f95a29027638be91a91043e72619d17a13664519c84dc0603c2f4feba215a09588fe297b695c5cc094205a47da4ecb7b5a052858cd69e95a6a47a1f200921ac9f0d6c9a49377211a45b4d0166329f57dc6650da3176c98384009a31bd1d215b153c2e0841682fe13e1eb34d79ebbc992b36abe49cb607f6b2399838e537eac7485e900f900956337a867f538943b193ad75f6d442a50c9774f198298bfa29422f4ce6ab1840462c2785e1590f1bf9a8f328af5939f4f1ddc83965abfed610b530c9f371a951311fad05c136c1fa567e09f7d4f415dd14ae0dd02a738d12808980e5fc6b9f5a472df9d9ca6ac2f9a13ae45652f944267cd8c3c27ba896395b503b1317a243a17675ec40e8561d8bdeb70eccf82531c5bfc388bf5d05c3b1e355348b5fbdf9642c2d76285fb0110adecc057c554d1523b2a57afe63c8c2c63c4bce9d96b7edd129a079d4c73ed05183d93e10f3d0dbc8e7264d6b7cbcae37e0fe5529a8ffe12b36a1fc6da3f7341eb2ba8b4a3d6d052e3bb9c84533f40e57e6eabaeccb6a3e67dacae63c4fc37fde7d83ea84d902447842fbf26a8f4c45298d0d8fa2ea688db0b80647b0c9572d06ff9ef5577a6efcba4f780ec5d3858b6026b3d60e39a41d597b1721065657d005f7e6b39b448969d14bee07cfec36f307bf01e58fb3fa8cea4b97612c2e3585c440e407880e1f3aa73d5f46d545218e6d05cf70a2b965ec18847af6056cd61587323b3eafe0bf443e06bc8b2030b5a54bd0db406d1bb5059443ae4274cf3cff5fe19cdb54f0277933453097bf311a901f75554a77341107f2cbc0a2682e54166d5bc8d3011cb3324c2efd05c7733d8e5e0fd08233fccb62ba612f475ca92eb19e5545179b6e092e8bb2009d2e65e9a6b5fc03005d5bf829cc9cc3a0bfc9de1f38e6246dd0f309c4873ed5f115d88677e8f6a0c918ff1121bf746759c5fe3d9d3b0bcc6ccdafb68c7c5ab3c1fd82a21a01800000c6c49444154a16edd456c1c85b68896434348d517f174006a63186709c2bb0c7ba8d0e72a5eed32c03d84b4b4f1c911af67bff5e70c7158b1fa6b78b6126415ceb297361a013663f057315e39d1e4abe49fc8da5cb33fe25465dbf4adf2df006963f8bc97b25b96dfd596d2cec1905e9a758b9e5e5d8671f7c7482f8404a9f05b050992a71fd78f5032ccf9f33f28d8fa34d7ec8f89fea7c09a1daf76e91ffa3ac669e73f1dafd3f8789a5b4f2ac76bff129ea4a9aa7821ab419b39e8004cba1729798f91f31731fdb87e845331ce6b7d77939ec6cd83bf846a6fe093ee3ea5b588228a28a288228a28a288228a28a288228a28a288228af8aca39859f13380783cee8693cb7b26c3bd5634363616d32075218a04d9453167c810e7cd7dfbff549071a0fba364725fbda9e8b8b1139bfeb4b967ec4a983364883364ee5c2b14da965ace3f3f346cf6ec6ef38c2e46b2ed8298d050bff75bfbf47f5455061be5e42b2736c995939a1cc43900f411943f5e3ba6fed41dddcfaec284868bbef6d63e03debc7eccc823f3af5fd730f2c4a53d63c9ebc7d49fd45d6def98b4a1456c35e2a3475759d2cf2072c75513a75e987fefca1b7efd06307c7c437dcaf861cf9f095835df14d8c78643ef14de317d04f08c74faecc32d457105d985a02011277d1de08e5d8f1cf9f0ac366dc76e753f44f642e0eaf1bf2e880dba6ae2d4df1874afab27debc61728d2e429120bb10c68d1c598e9161a85cfe69e5bc929eefe1e8dfb657bfba13713fa1c59eaabad1c422974f6c7abf3bdb2f6eb176214462e66c8064cafcf5d3ca35363626592fa99e824c6c68e861a5cd75bc9264c3e4c9d97c0273860c715efd529f58ac23ed5c3e71f6aafcf2edd1355e63e3ccec1666ca2597c412916449e95ad3563f6d5a2253f6d7975c125d6d6d84cacab67c4dda0d0d0d152aab4d47a4efaac6c0a975ca2597c482b2eb82be663171cc98f27dde7d77ddd0b9be33e880c58b9da595d13d405ecd7f17019d3a7264a4231a0d5f3679729b80c6e37153d6d6166b2b4b3ae31aa7adc908f4937ff9cbd254a823da7b796af5c604fa783c1e66f5eab27211a322ed63a64c590b70fd65f54d452dd62e84f163ea9f403828ecb97b653ec4ce60eac89191b55166a84889080759cb5e6298347662d30d131a461e623127a33a18e14023a97daeb8a179c5f597d5dfaca2bf10e52311a9b962e2d477af19336a7fd7700fc8010abf4d462bcf67f9f268b844ce11cb412a72b6c265574d9ada0470fd65a32601f5002a3a7eecc469d78c6f1875b95a3910f407469876e5a4a6ab33fd8cc7e36eb863c5720fef885f4d9cf1f2f831a38e41a8453817e503449f164c15c2ffbbf286a97fb9be61d42ce05b890e39864a6b23ede64285a3040e77347d50c3e4198bae6b1879a251694618a8ca836327351504f58dbff4e22330f642853705f932d88341ee42098991d1c52dd6ae04a3872abaa2ad6c4da74376e3f1b8692b61a11867c1d8494da725a23dbf25f05b9009e31b46fdec8a89d3feb5ff3b8bae15f8832015a164cc193f66d47556f931ca5c15d95755cf017085c7adf26755fea56a8786932b7a364e9bb67aff77164f8fa49d4b40178bea1e00d7358cbcd5c2f156ec308b7aa2e6826bc78ede2319ed7963c4ba171ab4dcc25ef97d8d2456ef0d526e9cf072806449cf475d8d8c1770d5306becc4691726a2956724229573832a87a050e6baa9c6c669abaf9834f5066bed05083dd438ee8486fabd05331b18ab2a770b32f8dacb467e27d3de7597d67f1dc73e8ec3f5574d6a8a2763954345e431a0418457ac23038a04d98520484cb6705b1c695f71064aec8a8937df0ad0d8d89876f1268bb25a826ff7a173e77a6248a24a2292ac12f8fe5593a7ee317652d3398aa655a4d7f886513701cd574d6a6a14a3c30c1273d39e93a95fb6766d5a55d322b2fcbacb461e2e98f055939abe7cd5c4e9b78bc81588ee2649dba7b1b131dd56569640c00805da27abf6480144bd74d057bbae6495075269d077836bc9cc564d2d950aabdbcaca12fef8a01276d601a8a4d6284cc4dad3ae9cd4f45bd4dea4821a357dc1dfa619a3f582bc337642d32b99f6ac35b3052cc2b7aebafee6c54582ec4a505d8d487f703bffb9899c83484106f586c93316817d1fa4f78da3475701a8258c0858f30d45afca564710b4afc2ee574e6aba1640d3260c90769df533b38455f43d23f24b8c8ec95cf4d4cc4705c7a8af7a5eb9b20c15d46a416e65517eaaa8079a97b3c03528d8b4d93097b1913284b5f9328f6389a9e2a1a1a35491b193a73d0a601d27e467d3f6b3e9348d1c19b6e841aab620138f6bd6bdad228a1283a2166b97820a4f88128976389dce6366e1bbead90d344016f950552523a95bb531858f8043931d2c8440185615947dc5333767ea8ab037c8eacc373dc0a2fefdc58888288350797eec84a69c9240d53f1d146f15801bb2bdad111172c767c4e3179489680c249d4c84b25bc89275a58280aab7e139984a4c6ce1d9575e5a7b20ba4c55c789498fc85c37d6abf4dfd35b04104d24ac605620e6bf0ac645ca7613d451f113781709b20b4155a62160ad8cef6c1d5175c4381b2485304215b06adcaf7fbdccffdb44451145d38dd3a6ad0698d8d0d043555144c2eabc997da6a3bb5b65a5d1b202bf2f151c154ebe72d2d409f9d75d747f2c1de9b45d0920863e02a8e45685487ba85ed13f8b6a28124e6409629d3617206c74c3835a45c39a9f4b0d50871ea8f451e47fafbc61465e9cbf54004435fc11c0b0d9b35388fe512032bea17e5cb63d4d8f56e4deb1139bfe004582ec5248c52a1f55abff2b468e9dd0507fd4a6cacd1932c489c7e36100413a4428281b8fc78daa7e41e0ce9c6f9344157653e5ee6c415d572e2288ead2bdde7f3f3bd9d46a6f31bab63dba263b397b2d5f2eaa54a8ca8cf5fb638d9e8ed17f1ef0fe920ffda69cde02622c2bc1570523ba97c1795dc12452d16c7e34d11e5651d2ee8607fb888a42c1e99fb8d89e0262c414a6abb55a024abef66fecc4a63fa0fa7355ae1a7fd9a8e7c78f19f5a4b1a4c74e9c7a62a64c9120bb101a1b1b9302972a8a55fe3cfed28b8f58bfcc75632efee65bfb0ef85d78dd8aef01287a2770da8486fabd3365421dab8e059627623df357a25211b87ad2b4a732173c8f1e8a0822af65ec12001671b184062c4e67bfd597f75a2d46a4d4116f4e7e7fc65f31ea60a332149131b967784641adb1df9838664cb947c755aa3c628df79188100e7bd92da4934ea705412dd9491b8f9f150d7e5d0ba634bf3dcf9adea058635fcdbfee3b72165a356e1c3dba4a91a1c670ba1867683229c75f39b9e9a2fc324543e12e862b2737dd3f7eccc5470ade63087f1fdf50ff94a8bddb2231417f845ac74ba74fbc7aca4cfff01a8febd5e80f2dbae0ba86facb04b5602fb79e777446b88dc7e346db97f744a5e0380b75e821a20e42810c63b08b30e60b4b2ba3bfbdae6154c811d3d0c18a45a85a1573e7848691977424dd2722115bab1e37abca51636fb8f9994c7d312c108ba05c9b968e71587964ece4a631f178dc8db4af58425aff67c2a5a32ef61c4db645cb1e08b52fffa7883361fc65a3be884a1fda7509701ea26f88ca51d78da9ffbd880dedff7f1f9dfe96d8fe0a4b931d6e814150d1358230feb251ad0ad62a1725247da82083557966ecc49b367a145c7105d9053176f2cd4f44d26e99a27154ad624e1391c34475d2d8c94d5fcd9203183ba5e9156bd3078a31f30c8c1095c38ca4be975f665c63a31a91bfa1529059532d4b44f5979ee3de977fddd1688baafe1961612adaf3f42b6eb8f99d1eab7b0922c6f5d2dfb16a8e8b84bc56558eb521e7b0b1936f7ea2a0ff13a62d51d53a15794c30e3c74e6e3a017c15b4c59e20469e52c3d76349777e636363da313202d5fb05898ac384b1939ace03b0aa9759e16f62f481b193a69d3274ee5ccf629e17752e1eb076ed9afc36c3eadd6fadfd2dc2a3112ff4a35f4d6e7a33ec85ee017d5e207e7dc3281d7f59fdb3e3c78cba6e4243fd91f1fafa4a28c68314d14588d7d7578623ac48b6db8a8c90bfb363c2e5f5433c4f8762cced466d950ac7a0fa134162167d296cbd41c52d56115d0237647b23c6a3d78eee49e770fde5a38eb3566fb86a72d37e79977f0b9c39be61d42f45e546cf75ce2a6eb18ae81288a10f56db4bd64577faccf5f178dc60b9c7a8ac7f3013006327364d1125e9596d2f12a4882e8151d317a31deb4ad6eef4048975acd81d11e389463776ffda5f5eb40f46c2ae98bb8a0429a26be0e800b146cbda623b3d412e9fd8f4be557dd420a3c78f19f97d0d64f1783cee4e68b8f868e39afbacc739974f6c7abf288314d125103569157b5f3899dc25b2aaa4621d2784db63a7238cbbbe61549feba19df615fff190475cb4eef21b9b3e04f8fff5d9f36ed69c8e2f0000000049454e44ae426082";
#endif

 return logo;
}

QString icono_smart()
{
   QString icono="89504e470d0a1a0a0000000d4948445200000096000000640806000000645bc9f00000000473424954080808087c086488000000097048597300000ec400000ec401952b0e1b0000163949444154789ced9d7b941c559dc73fbf5b5ddd3dd39319262f022190802b88ba088328208c991930bb6765e528d13c08888f4812440517ddf5981357511751209909f1b1f8405454547ca0928751240f12920022418246115602e43d33dd55757ffb4755f5f424f3ece9c963a8cfe13099e9aa7b6fddfa9e7b7ff7f7fbdddb909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090f032420e77032acecd6fac3aaec619a7593f078067da2528ec786eeec6f6c3dcb2971547b7b09635b8c73ae953ad712e34c22588bcde20a345c044978880aa027617e81a55fd714aecefb63fbd6e2b8bb087b3f92399a35358375d94939ae0866a473f6c0c350e4a4a54530e22580441508c015440c0441a12114041ed8b85c07ee2d982f735e66ef40eebf38c408e2e61dd76c114dccced2017805401a028828046020223966a1390732dae5140106cf833d4951ab1d1b3ebaec0ea9ddbe7acf920821ea6271b711c1dc25ad23801713e8371ae0ae7b6584c3d110a2994889236969a54402ee563241eb1e2114c00552322d6da9d827e6cdb9c355f3e340f35b239b28575f7650e3b5ebc12e37c1511b0aa1891583bdd0885a445e51cf09988a5def5a8712d464a8415db6356558c88557f433ee06dcf5cb9f6efc3fd7823992357589f6ba8a3ae7e1d9853bbb55215602f4a3b681e91346835300a315d57761bd534b4b550449431ae47adeb87967d89a11fea5211d520aff67ddbe7acbbe3503cea48e4c814566bd37b10d386481a651ba277a1fa5be069acff3cf357ef3fc81ebae9a21c55fe58c4998c703ee874c49c41b701ac6b9a4c9b80719902d994452203bf282cc2ffd9c07eefa9396b664437260c82234b58775fe6b063e7d7305c8ad56f81b4327ff91fcb2e6f69d3442c7311e79d08af8c7c0f910f0240a94f7bd4a73d4cb45814096753294e8dc15ac47beb93b336be50a9c77c3970e408ebd669b5b8de4a94ff65fe8ab68a97dfda34159caf60740a624c68af21a890928013aaf3d10a32121651e7a855d017768b77fa7389b806cc9121ac5ba79e824303f98e9f72ddda8e61ab672186b18d67914add8338934ac525a28c4be73926ed13bb256261891141edde4e3f38fbcf573cf4e4b0b56f047164086b594b1d7397ef3ea475b636cfc3c8171193295d6dd6ba0526640ba19355006c7104536b7d9fa0f94fb31ffaed216deb51c89121acee08775f162ed4a67fdf329c86f392c61ac4fd2946de0ca2880a2a641d9f13aaf3a44a8405205643cb5efdc63f26e2ea93c3272c45724bcf1d579571ce49895e8848834126239a13111744c45a4f8db663e519b09b3db56b44f4c17fbc7bdd762a29b8254d57e2387794faca1c2c53721da44c18228afe435455515bf0fdd73f75c5864d156bc308e3900aabe66baf1b9795dc5c47e412902946643460a2257ebce28f627ce1ea2d6ca0469f69bca0db8be559d04d81da3bfffeee353f1f72e36ebb600aa9f40a8c33051b3a5a459413abdbc9a56cd12926842b4755f57c2f386feb150f6d1872dd23906117d6b825a7d798aad19781bc478c9c2f51ada284863382a82d8aa828ac6e0d540445553006d4a26234be03b5b653859f69107cd72bf8f70d2945a6adf92ec4cc206e9fc2c4aa4eead23ea1911fb92400ac6a418273b6ce4cc47520c327acb637d523e97969a39f0a708cdac8672e88015262711d25238aebd82878aca12fc948d1ddd4a3b04a1ade15f32ba6c858abf696bc166e7d6ecec6bf9515586e6bb901e4b3a141af0ac8f1559dd417578c6191a1cda5f9ce82bce24f4908a81bc323acd6a65b31e603884913ba867a0f1ac72f09c81a9f51a9808ca391273c5e990d4c5868e8d404c0da00e58f79b5ef79e6cab5eb07fd0c4bdf7c3ea41f20ae4e9463d379c65579c506147d5dd89d781d931e99f3c8fe41d73342a9acb05a5b3e80611122e3b1124d750344bbfee118256b02eadc00d768347a0d6cc422fa4dadaa3122aa8aa29b7c1b7c7afb15ebee19e4f3bc0ae1018c191db7715c26cf84aa42b71135b4f9ed737b3a3b4fdffeee2dbb0655c708a532c26a6b3e19b803e35cd863e6c1a0e90a1a573b3eb5e9808cb15dd324031016617039b6c70002f46155ff437fbe7cfdef06dc945b9a8e25e33c88c8c9b1513f369de7b8ea3cc4c3b0c6a937c1c3bebbefdcc7a73f5e186a0f1ced0c5d586d2d3341ee8cd20486676a55a8723c46a77d5ca7eb6f5d99a17d08ab18ffebfa2cb0c19a02dedbff7af9c6e70654ffcd6faca22af787d215e3e8749e8955f9d0c7559c16156bedea4766ae7f73859efca8a57c212cc430a1e57e90a66e1904f11cd11ba12ddc47a25e6ff785b7e51c9f71192f723bc8e08515f9a954d5b36a7ff8d4536b670d28f7fd8ec62c9da90710a7211e51ebd3054eacee0c0d48210a5cab58d55f6d99b17edaa09e6f84519eb0da1a5f03eebd1833a5f8376b0384cda09b50d906fa3c56f7a0a2a4c8a28c41e504e055889c8930b1f8b6fbcc082d257ca1462ca3d31ea3dca068d4876e80ae87ea6bc40adbab2a0651d517026bffe3a981e65e2d6df90562fe255c2d8ad4a7f39c94cb470fa060558d231258ffe62d33365c3fa03247208317d692a917e238ab400ce81e94f5a86de5f995f70e6ad74bdbb9e3b1d5efc2987781be0623a3a0244bb43f14b28ecff84c81549cd73e086195b8315445446db0268f99f197d96bb6f7dff6967b31e6adf1b4188f5cf13e0d23aa22227ee07d64cbcc8d5f1a709f8c200627acc54defc54d7d05b5057c7b032f044b58b4da1f722b9635b804756f07b919e31c1fbd30fa1ec5b468678d750bd41e909530606189a2163582a86ae013dcf0a7d90fdddc6f9bdb5a7e8c31ff1e8f5cc7b80526d77486c2321447442fb0573f32f3a1dbcbee9ba394810babadf926c45c0fc13504c1d759b07a5fc55ba3084ba79e859a4fe23897447feb7b9a8c12f6b28e6542551e27f4eaa332086121ddd293157dacc3faeff8cbe51bb6f6d9dea52ddf41ccbbbaa6c50253aa3b20ca90165555b5d6b77ae923b336fcb4ac3e394a1998b0da9a9702afc2f36773edea6786b749118b9bcec2713e8330addb468ade88364c4cc8e68bb1bdc10a2b76634481226c60af7be2f2755fecb39d4b5bbe8e982b4aa7c529351d5d7544ee0e4f83376d79d7c6df97df214717fd0babb5e9b320cf327fc5e243d09e8359dc7836a9f40f8093fadffa05288c4ee7a94f87dbbd344ae21bacb0622f7e6083cdf9c0bce5e9396b9eefb5cea5cddf429cd9685844ad5be015359d61e8279a1a5595bcef9ffde8ac87370eb94f8e02fa16566bd3a7d0e09b2c58fdd4216a4fcfdc7d99c38e97de81317782a4fa6c75bc9fd0b19c5095c781d0bb315861853f15b182a5e06970edd6d9bdd84a8a707bcb1d2057108d9db5aec72b47ed0fa305128e666a35df1904af7b6cd6c62786d41f4701bdbfa225532f27b0ab0ed9d43710da2e1a0ffa458c99d5af875fc31dd1c765f3e4524197af29fa7840c28ae632d130093008f4e741e7ae195bdfbb756fcfed6bfe3262de178eaa91b86af61777a549e88bdbdfee05678f7471f5fc6a6e6b3a87cceec738524f6859dcfc761c733b86b1587ab7bda2d16b7c36ca65573960e41aa0b0223788189140ed4e55ff92c7676d7ca0c73a5b5b6ec798b97171b994c769b5ed51e64614eb544b9ec2995ba66fd95ca11e39e238f8852c699cc082d5ff7718da3238166238b66539c64c05e22cc01e0416bed0516e81e3aa0a91fd549235315061d115dc168358ab5f7aecc975d7f7e8bb6b6d5e8231f3e3ac44d704bcb66e1f29d170f48a3cf4beef9dbf61c6e6072bdb314706a6db6f0b1b53ece0e8d8e2b408cbbce54d583b2b326b7a991843ffc35e2fcdf67d596c141a2a1b1111448d633efcdad3ce79e255779e79d241d7cc5fb1006b6f461554d5b30e8fee1e45c19ab849a22aa41cf7370ddf6b98517e632accad534fa95451c313343ed4dcd2f81a32e99f841908f492ae138e5c8e582655779275e2638d063962c5abbc686ac4dabc87bde6f1990f7de5a02a5b9bfe1be37c8248f98e584eaddd4fadeb87467d94a51a6870c3fae91bffa7e2fd32501636a618efdc48bbb3888fde5f919cb29121ac98d6969fe2c8bff56bd52b4caaee24970aca17567c9d4545acf8d6fec0eb70afdafade07bb1bf6ad53afc7a46e2a898b726aed5eead33e12ba4214110902fbcdbdd2febe439e72b3ace5447c7e8d671bf9d0ca7f54aad891252c88ce7d70c2d1a31fa37e4c26cff8ac17b904ca1356f84978fe8355bb2f40a73e76600efcd296e920df8b578b88725cb69329b94e102926255aebffad03dbb079fae61d15ed93de887727f985a95cb3fa37952c7ae4090be0f6c6b3b1e9e518a9ebf33a855ad763627567bca5b07c6111e7e9abf5555b1f9db1fe83ddea5ad23c0dc7dc17397801919a94c7abebf6e318052b6a8c1555f256834faf99bee9d315eb8f03b9f98da3a9cadd8d719ab1f6bf98b7fcc64a573132850551e6a7f925625ed77b3828b4bb72298f49d5f9928341c24fcb11568933f4a1bc5798f9f89c4d5dcee5b6c63722e9fb1039a66b8b99e5d451fb1993f111d5a2cf4b35d8ecfbb2606d25c340375d942367e721f2b9f0fc8ae076e6adb8ba62e59730728515d3d6fc2bc45cdc77f221a424e0e49a8e3005a75b7ed72085151a6b2aaaa8a804d8abb6fc71c3378a6e89c5cd63303c8ae31c57bab57f6ca6937faae920e58471216345112b81d5df1702ffaac9cea9dbbe3ffdfb41597d70d34539aa74264697609c340041703ff3575c5c56790360e40b4b11da9a3f863137a2bd395363312927e7daa315a3942facf85a14030436d8dcbe3f7d61d1b0bf755a06d7bb17e35c1c4e8de13c6cc472625507936af2841b782daa02620390676c60db76ec695fdaabe7ff00def2e3d7b6ac7fb1fee33bf3d9376034577cddd62e676cfd34ca11eac28586da5f65fa3bbc65e40b2b6671d3a5a49c7bfa0c644746fdc4780fa1084316563115daeef52dd76d29754bb4b57c0c91cf96ae1801324ec009559d1c5fd58913cd8c12fbe9c2b49e2711fd83b16c53d57fa8a1dd28ae41c7388693404e7da63373fa5ff6e746edf753e14dc5fb836f71f58a39e574e1c9cb1aea9ef6ebae63feca4ff677edcb475800b75d7c1aaefd2d62c6f567771d9bed645cc6a3343dbfdc114b95f0d3305b62a5efe5af78f4f247c3186c6bf3eb11b9079113ba46d47804b54ccc76323eeb519d0ab7c285c56bbc0d1c43e88b0b54e80c0c2fe65db6edadd6823a5d8d266a4460bfc082151f2da7ebde7fef71d5f7ed9cfcb3bf3d7d710b8b16f59b29fcf2121684d350caff038e39a53f718d723d4eaa8ed25f90a1092bb6d9a27f7b9eff814db3362e2b56d9da72278e991516a75d0223fe11efb5f41995f28bfb2d3bac61b7e7b2d74b11685c79745f743a0eaa0502ff12aef9cdafcae9b286650dae53277f5aff42dd5759b06240abd5979fb0201297f71d1ce7d2fec495757c4ea969c74436d7d085155a4de1aca74f0412cc7e787a94a375dbd43348397761cce9074dd7038942f5f436ad5d85f566961bffbde087a71d5763d21b7efb7cfddefd73579f36d0fb5e9ec28a696bbe05e35cdb7b188862107972ae832ac71eb41b7bd0c28a3f8b76f380626d70871fd84f6c9ab9e95900964c9d813137609c33c2e207bac124b6a5142cdb403fc9bce57795db3dcd3ffce769ae911f6dd9559b7d6eaffb063eb872c04715bcbc850551c8c5f93c624c9fd769b862ac7103a4e4a8f9b285d52de618ce77d6b71f77ab32ad0fbe2d5a3d2e6e3e17233762e47528757da7662b4007e8d328370e45508dab1a53ee8bbbff33e5caa21d1d2e1b5f1875270b565c3e983212614178f0ade3ac2c865c0e22da9d8132211b9edd60635f17302461111e788286f14aabb63dc02edcf0ce87bf50ac7e59834b507b32ca5bc1b420f24a0c55a8b1a87d1eec83203fc2fa6b7b3caa7c109c77d799c75767f4819463a6ecf78cfe6ec7313bf152c773ed2ff383292711564cb8625c8398637a75474402a8750b4cc9e54b568c151016822999ca54f57945bfb9978e458f4f7fbcf23ba20e6421a6e98c336e4989996b44d2a8ea6f76d44b679e8bb966e5fd832d2e115629cb1aaa09ea7f873167f567d4a79d8053721d641c5b74130d595825d7a28453a42a56ed57d4e76beb666c5c3f94d1a827def4edd7d6a7b366a611f379c7482e3a9e87b53beb645767ea1bcc5f7e6539e526c2ea89d6e67b709c4bfbbc267abd27d7eca736e5472b46a998b0baefea8ecb09f6f8aa3709f60e6f67eaf98d657e1d5ec3b206b7bab673829b763f618cbcdf445f98a046c4a8f2d8ee1a9e69cfeee21ff78f29f73b1d1361f5c692a6eb48a5bed0dfc8852863dc0227e63a8757581a07adc1aa0d04d9a9569f54ec2f02cfffe58bf51d8f3df5af4ff56a079d77d799c7a7dc609a23e63231e62c41c71811a7b41e83b26d6f953eb9af46b0fe19cc5ff948b9dd9708ab2fda9a2e03e7db18717bbf281458b5e331a5a6233c8d3032ec2b3d6201b12f4d0d48b7146b5504762aec16e850516b90344a4e8c1d2d48b6f88d1b61c6b48aa1f8bda046943fefadd2adfb6a04eb7d94f9abba160f659008ab3f6e9b7a06aef36bc48cef73f4428a2e8963327e240886475871b9d1bdc59053c93dddebb194ae46bafc2a1adda7fcbd3dc31f76d782dae5cc5b7ed1107b2d11d68058d6508dad5f8f9857f7be62a46877d5a63c4e19d581c8308d58544e5822ca5ff7657862cf2805fec6bcfb4f1a525f1d544742efccddd8ce98fa33b0f64ba082d59e5766d13cb3c7777964770dbbf2a94a2fe22a862a38a26cdd53cd137b6ba350a89d5aa9f293116bb0b44e9d8371bedced3b787a24b4b58e890cfb4c140e2a1eb37458472cc52a3cb2ab861d9dd9301fccfae7337f55c5f63826c22a87d60b2621e9b518737c9f3bb1219a1e95c9b976c6647c8cc4dfb221875e581ada539d8161dd8b75788113c64803ef2ae6afbaa3925d9408ab5ceebe2ccd8e97be85e34c1fc8c8054aca2853aadba9cff8d10b9743262ca27bb6efcff2e4be1cc5e3d283e03ae6afe8fba8a632488435541637bf9d94f92e22a93e0d7b288e5ed54ec0c4aa3c63b35ee87d8cd4345cc24a19cb6ecfe1893d35ecf15c8aa1023ff8080b560ccb519689b02ac1b2c6b158f72720e7519a72da235dc97b19e3332997e718d78bbedd955e0eea1da4b0a2c44003b4fb86bfb667f97b7b55fcb6c321d4fa43f655f54522ac4ad2da742d626ec68833b0aeedf2de8f4fe79958dd49c684765097b0a45f61c51b416267a722e47de1c97dd5bc54c84455c53b821482e0dd2c58f9f54a3f7e2989b02acd92c60988fb4d8cb9a8f832fb24b6c100515c516a1c8f71d902f5191f57c26b6287eb81d39d893eb30a7bfc14cf756478a9e05250d3552ed1b06655a1701ef356afadec431f4c22ace16249d33b70e48b8833a9f723967a40bbffe21a4b9563c99800d7107d312758157c150a56e8080c1d81831673de4b8ae8ca7b7f18b19770f5ca43f22d6589b0869bd6a6cf22ced508757d6e9aed132d1d7dbad35b91f14ad5da00d5af327fc507caacbc2c12611d0ac20daa9f07f3c1a29d53b6c806405154c17360a7326f55dfc78a0f0389b00e25b7be691c4e66368ef9142235406c810ffd3d68d18a076b5f42751ee346ffa0acddce152011d6e1a2b56936c65c817221c684e7290c76248bbeb8a0e8b04237a3f6abcc5bd93a2c6d1e0489b00e370b1b538c4bcd41781fc69c866a3522e9f0cdf4f17ac240781e7427ca7df8c18d5cbb6adb216a75bf24c23a92b8755a06af631419773246cfc1703ac8642cb5e1aad2ee049e46ed1694f5b8ce333cebedafc8f71925242424242424242424242424242424242424242424241cc1fc3fbaff1d3e07b45ff40000000049454e44ae426082";
   return icono;
}

bool testNIF(QString a)
{
    QString dni=a.toUpper();
    int numero=dni.left(dni.length()-1).toInt();
    QString let=dni.right(1);
    let=let.toUpper();
    numero=numero%23;
    QString letra="TRWAGMYFPDXBNJZSQVHLCKET";
    QString lletra=letra.mid(numero,1);
    return (lletra == let);//devuelve true o false
}


bool testNIE(QString a)
{
    QString dni=a.toUpper();
    QString pre=dni.left(1);
    QString prev="0";
    if (pre=="X") {
        prev="0";
      } else if (pre == "Y") {
        prev="1";
         } else if (pre == "Z") {
              prev="2";
             }
    QString numero=prev+dni.mid(1,dni.length()-1);
    return testNIF(numero);//una vez tratamos el NIE lo comprobamos como un NIF
}

bool testCIF(QString cif) {
    int pares=0;
    int impares=0;
    QString suma;
    QString ultima;
    int unumero;
    QString uletra="JABCDEFGHI";
    QString xxx;
    QString texto=cif.toUpper();
    QRegExp regular ("^[ABCDEFGHJKLMNPQRSV]\\d{7}[0-9,A-J]$");
    if (! regular.exactMatch(texto)) {
        return false;
    }
    ultima=texto.mid(8,1);
    int cont;
    for (cont = 1; cont <7; cont ++) {
        QString conv; conv.setNum(2*texto.mid(cont++,1).toInt());
        xxx=conv+"0";
        // xxx = (2 * parseInt(texto.substr(cont++,1))).toString() + "0";
        //impares+=parseInt(xxx.substr(0,1))+parseInt(xxx.substr(1,1));
        impares+=xxx.mid(0,1).toInt()+xxx.mid(1,1).toInt();
        // pares+=parseInt(texto.substr(cont,1));
        pares+=texto.mid(cont,1).toInt();
    }
    QString conv; conv.setNum(2*texto.mid(cont,1).toInt());
    xxx = conv + "0";
    // xxx = (2 * parseInt(texto.substr(cont,1))).toString() + "0";
    // impares+=parseInt(xxx.substr(0,1))+parseInt(xxx.substr(1,1));
    impares+=xxx.mid(0,1).toInt()+xxx.mid(1,1).toInt();
    suma.setNum(pares+impares);
    // suma = (pares + impares).toString();
    // unumero=parseInt(suma.substr(suma.length-1,1));
    unumero=suma.mid(suma.length()-1,1).toInt();
    unumero = (10 - unumero);
    if (unumero==10) {
        unumero=0;
    }
    QString cad_unumero; cad_unumero.setNum(unumero);
    if (ultima == cad_unumero || ultima == uletra.mid(unumero,1))
        {
        return true;
        } else
           {
            return false;
           }
}


int isNifCifNie(QString dni)
  {
    QString cadena =dni.toUpper();//convertimos a mayusculas
    QString pre=cadena.mid(0,1);//extraemos el primer digito o letra
    if (pre=="X"||pre=="Y"||pre=="Z") {//Si el primer digito es igual a X,Y o Z entonces es un NIE
        if (testNIE(dni)) {//llamamos a la funcion testNIE(); pasandole por parametro el dni. Devolvera true o false
            return 1;//Si es true devolvemos 1, 1 = NIE correcto.
        } else {
            return -1;//Si es false devolvemos -1, -1 = NIE incorrecto.
        }
    } else {//Si no es un NIE comprobamos si es un CIF
        QRegExp letrasCIF("[ABCDEFGHJKLMNPQRSUVW]");//Estan son las letras por la que empieza un CIF
        if (letrasCIF.exactMatch(pre)) {//Si la primera letra de la cadena coincide con alguna del patron letrasCIF entonces es un CIF
            if (testCIF(dni)) {//llamamos a la funcion testCIF(); pasandole por parametro el dni. Devolvera true o false
                return 2;//Si es true devolvemos 2, 2 = CIF correcto.
            } else {
                return -2;//Si es false devolvemos -2, -2 = CIF incorrecto.
            }
        } else {//Si no es un CIF comprobamos si es un NIF
            QRegExp numerosNIF("[1234567890]");
            if (numerosNIF.exactMatch(pre)) {//Si el primer digito de la cadena coincide con el patron numerosNIF entonces es un NIF
                if (testNIF(dni)) {//llamamos a la funcion testNIF(); pasandole por parametro el dni. Devolvera true o false
                    return 3;//Si es true devolvemos 3, 3 = NIF correcto.
                } else {
                    return -3;//Si es false devolvemos -3, -3 = NIF incorrecto.
                }
            } else {//Si tampoco es un NIF entonces no es un dni valido de ningun tipo
                //si no es ninguno devolvemos 0
                return 0;
            }
        }
    }
}




QString CalculaDigitoCCC(QString Entidad, QString Oficina, QString Numero)
{
    /*
nsiString __fastcall TForm1::CalculaDigito(AnsiString Entidad, AnsiString Oficina, AnsiString Numero)
{
     AnsiString Cadena;
     int nResto;
     int aPesos[10] = {6,3,7,9,10,5,8,4,2,1};

     Cadena = "";
     nResto = 0;
     for (int nItem = 1; nItem <= (Entidad + Oficina).Length(); nItem ++)
          nResto += StrToInt((Entidad + Oficina).SubString(nItem, 1)) * aPesos[8 - nItem];

     nResto = 11 - (nResto % 11);
     if (nResto == 11)
          nResto = 0;
     else if (nResto == 10)
          nResto = 1;
     Cadena = FormatFloat("0", nResto);

     nResto = 0;
     for (int nItem = 1; nItem <= Numero.Length(); nItem ++)
          nResto += StrToInt(Numero.SubString(nItem, 1)) * aPesos[10 - nItem];

     nResto = 11 - (nResto % 11);
     if (nResto == 11)
          nResto = 0;
     else if (nResto == 10)
          nResto = 1;
     Cadena = Cadena + FormatFloat("0", nResto);
     return (Cadena);
}
//---------------------------------------------------------------------------
[editar]     */
     QString cadena;
     int nResto;
     int aPesos[10] = {6,3,7,9,10,5,8,4,2,1};

     nResto = 0;
     QString entidad_oficina=Entidad+Oficina;

     for (int nItem = 1; nItem <= entidad_oficina.length(); nItem ++)
          //nResto += StrToInt((Entidad + Oficina).SubString(nItem, 1)) * aPesos[8 - nItem];
          nResto += entidad_oficina.mid(nItem-1,1).toInt()*aPesos[8-nItem];

     nResto = 11 - (nResto % 11);
     if (nResto == 11)
          nResto = 0;
     else if (nResto == 10)
          nResto = 1;
     cadena.setNum(nResto); // = FormatFloat("0", nResto);

     nResto = 0;
     for (int nItem = 1; nItem <= Numero.length(); nItem ++)
          // nResto += StrToInt(Numero.SubString(nItem, 1)) * aPesos[10 - nItem];
          nResto += Numero.mid(nItem-1,1).toInt()*aPesos[10-nItem];

     nResto = 11 - (nResto % 11);
     if (nResto == 11)
          nResto = 0;
     else if (nResto == 10)
          nResto = 1;
     QString cadnum; cadnum.setNum(nResto);
     cadena = cadena + cadnum;
     return (cadena);
}

bool ccc_correcto(QString ccc)
{
 if (ccc.length()!=23) return false;
 // 1234-1234-12-1234567890
 QString control=CalculaDigitoCCC(ccc.left(4), ccc.mid(5,4), ccc.right(10));
 // qWarning(ccc.toAscii().constData());
 // qWarning(control.toAscii().constData());
 if (control==ccc.mid(10,2)) return true;
   else return false;
}


bool IBAN_correcto(QString codigo)
{
  QString cadena;
  cadena=codigo.mid(4)+codigo.left(4);
  QString abc="ABCDEFGHIJKLMNOPQRSTUVWXZ";
  QString destino=cadena;
  for (int veces=0;veces<cadena.length();veces++)
      if (abc.contains(cadena.at(veces)))
         {
          int pos=abc.indexOf(cadena.at(veces))+10;
          QString cadnum; cadnum.setNum(pos);
          destino.replace(cadena.at(veces),cadnum);
         }
  /*
In this example, the above algorithm for D mod 97 will be applied to D = 3214282912345698765432161182. (The digits are colour-coded to aid the description below.) If the result is one, the IBAN corresponding to D passes the check digit test.
1. Construct N from the first 9 digits of D.
N = 321428291.
2. Calculate N mod 97 = 70.
3. Construct a new 9-digit N from the above result (step 2) followed by the next 7 digits of D.
N = 702345698
4. Calculate N mod 97 = 29.
5. Construct a new 9-digit N from the above result (step 4) followed by the next 7 digits of D.
N = 297654321
6. Calculate N mod 97 = 24.
7. Construct a new N from the above result (step 6) followed by the remaining 5 digits of D.
N = 2461182
8. Calculate N mod 97 = 1.
From step 8, the final result is D mod 97 = 1 and the IBAN has passed this check digit test.
[edit]  */

  QString extract=destino.left(9);
  int num=extract.toInt();
  num=num%97;
  QString cadnum; cadnum.setNum(num);

  QString cad_proceso;
  cad_proceso=cadnum+destino.mid(9);

  while (cad_proceso.length()>9)
    {
     // buscamos otros nueve caracteres de cad_proceso
      extract=cad_proceso.left(9);
      num=extract.toInt();
      num=num%97;
      cadnum.setNum(num);
      cad_proceso=cadnum+cad_proceso.mid(9);
    }

  int valor; valor=cad_proceso.toInt();
  // QString rdo; rdo.setNum(valor % 97); qWarning(rdo.toAscii().constData());
  if (valor % 97 == 1) return true;
  return false;
}

/*

Validación de dígito de control IBAN:
A continuación vamos a validar el dígito de control de un número de cuenta de un cliente de Bélgica. Como podemos ver en la tabla de códigos de cuenta, el formato para este país es:

BE XX seguido de un número de 12 dígitos, siendo XX los dos dígitos de control. El mecanismo que habrá que realizar es el siguiente:

Mover los cuatro primeros caracteres del número IBA a la derecha:
Ej: BE89 9999 9999 9999 -> 999999999999BE89
Convertir las letras a números según la siguiente tabla:
A=10	G=16	M=22	S=28	Y=34
B=11	H=17	N=23	T=29	Z=35
C=12	I=18	O=24	U=30
D=13	J=19	P=25	V=31
E=14	K=20	Q=26	W=32
F=15	L=21	R=27	X=33

Ej: 999999999999BE89 -> 999999999999111489
Sobre el número resultante, calcular el módulo 97, si el resultado es correcto, la operación dará como resultado 1.

Generación del dígito de control IBAN:

Componer el código IBAN de cuenta con dos dígito de control 0.
Ej: IBAN BEXX 9999-9999-9999 -> BE00999999999999
Mover los cuatro primeros caracteres del número a la derecha.
Ej: BE00999999999999 -> 999999999999BE00
Convertir las letras en caracteres numéricos de acuerdo con la tabla de conversión.
Ej: 999999999999BE00 ->999999999999111400
Aplicar la operación módulo 97 y eliminar al resultado 98. Si el resultado consta de sólo un dígito, insertar un cero a la izquierda.
Ej: 98 - 9 = 89; IBAN = BE89999999999999

 */

QString digito_cotrol_IBAN(QString codigo)
{
    QString cadena;
    cadena=codigo.mid(4)+codigo.left(4);
    QString abc="ABCDEFGHIJKLMNOPQRSTUVWXZ";
    QString destino=cadena;
    for (int veces=0;veces<cadena.length();veces++)
        if (abc.contains(cadena.at(veces)))
           {
            int pos=abc.indexOf(cadena.at(veces))+10;
            QString cadnum; cadnum.setNum(pos);
            destino.replace(cadena.at(veces),cadnum);
           }
    QString extract=destino.left(9);
    int num=extract.toInt();
    num=num%97;
    QString cadnum; cadnum.setNum(num);

    QString cad_proceso;
    cad_proceso=cadnum+destino.mid(9);

    while (cad_proceso.length()>9)
      {
       // buscamos otros nueve caracteres de cad_proceso
        extract=cad_proceso.left(9);
        num=extract.toInt();
        num=num%97;
        cadnum.setNum(num);
        cad_proceso=cadnum+cad_proceso.mid(9);
      }

    int valor; valor=cad_proceso.toInt();
    // QString rdo; rdo.setNum(valor % 97); qWarning(rdo.toAscii().constData());
    // if (valor % 97 == 1) return true;
    // return false;
    int resultado=98-valor%97;
    QString cad; cad.setNum(resultado);
    if (cad.length()==1) cad="0"+cad;
    return cad;
}

QString logo_facturacion_defecto()
{
   QString logo=
    "89504e470d0a1a0a0000000d49484452000000c80000006408020000004ce4e85c000000097048597300000ec400000ec401952b0e1b00000c3749444154789ced9cdf6f13c716c76776bd76bc4b7e19120707c71020490926ad485a52a19644d036ea0b155255aaaaeffd8bfa52a92f55914a8b4a79a91484a8445b0850c7492b1a20b48d15620c2e8963c7716ccfdc87117b573bb3f6669369dc7bcfe7c91eefcc1cef7e7dce993393604a290280ed46d9690380ff4d405880144058801440588014405880144058801440588014405880144058801440588014405880144058801440588014405880144058801440588014405880144058801440588014405880144058801440588014405880144058801440588014405880144058801440588014405880144058801440588014405880144058801440588014405880147c5b1f82528a3176f391d395354670fa94357ae8e8794606212493c9e472b962b158a9545455f5fbfd8661ecdebd7bd7ae5d7547de16aad5ead3a74fad36689a66184628146a6969916d83cbc1f1b6fc73db4f3ffd54d3345b63b95cfee4934fea5ed9dbdb3b3e3e5edb5c5b2febc8172e5c28140a7c97582c76e6cc991a364f4e4efef5d75f7cbb6118e7cf9f37df9a0a4ea7d3d3d3d38b8b8baaaa2a8a625a4b29a59412420281403c1e3f78f0a0611835e6bd76eddaa3478ff8f6b1b1b183070f0abb301b08214b4b4bd3d3d34b4b4b4e36a8aa1a8fc7fbfbfb9d6c28168b5f7cf18567d9099fa9906df05808a14020c00b4b51047196bf32954acdcecec6e3f11adab2f5b28e3c3c3cfcf3cf3ff31d5756566adbbcb2b21208046c8d94d2e1e161eb5b8c71a150f8f6db6fabd52ac698efc2a656559510924c267ff9e597c3870f8f8e8e0abf3e4248d3347e10e470bb4c1b565656befbee3bf6ba860d08a1df7efb6d6666e6a5975e7af5d557f9dbc2ba7b16969391822bbd4db08d288a9248244aa592876f4b298d4422d56a95ff68757555e8c918f97c7e7575956faf56abdddddda617c718dfbf7fffd2a54b4c556e4c5255757e7efeebafbf2e97cbdb120d30c6c964f2f2e5cbee039caaaa73737357ae5c4108edd4bf5bdf796131d88ddbec5dc0181b86d1d9d9c977541425994c3a759c9999e17f7c94d27038aceb3a8b7d08a1b9b9b91b376ea0172ec1bd55a552893998ad3c57d637994cdebd7b1763bc591b9697976fdcb8213be173a2218485315e5f5fbf7dfbb6f94437c5e8e828dfa828cac2c28270344ae9c2c282d0ab9b43618c8bc5e2ad5bb7f810cf46a8be801022b4aa50285cbb76cddb37326d585e5e4e2412756d104ec1dcad308ffc07d89e1c6beb608cefddbbb76fdfbe4824b2d9beadadad9aa6552a155b7ba5522997cb7ebfdfd65e2e97f98b29a59aa699ab2a84100b25fc6518e35028343232d2d1d1512c167ffdf5d7b9b939c479358c712a955a5e5e6e6b6bdbec37422ff2f1efbfff9e654ebc0d9148646464a4b5b5359d4efff8e38f854281774e9aa6ddbd7b37168bb999919f4868954bfb1b455808214551ae5ebd7afefc799fcfb72907aeaaeadebd7b1716166cbd0821e974baa7a7c7767d3a9d2684d82ea694767777b39bcb1276a7b4ef8d37de301f956118afbdf6dad0d0d0c58b17f91c4855d5ab57af9e3b77ce433c62ee6a6363439880bff5d65be17098bdedeaea3a77eedcd4d414d3b78d5c2eb7b4b4b477efdedad3050281f7df7fdf8d612e53bd860885562627273d848f13274e0843d2f4f4b4cb464aa935a4debe7d9bb781ad1562b198eda3a6a6a6f7de7b8f8fad2cc4572a156fd1706a6a8a6f248444a3d170386c1d93523a323262de3716253736367c3e5f4f4fcfdada9a87d99d70f92369208f8510c21867329999999963c78eb95f04514a9b9a9a42a190adc48031ce66b31b1b1bd668582a95b2d9accf67ffe27bf6ecd134cdac5a3d7af488cf6c0821a74e9d42a29b6b18463018e4e31121e4d9b367751d064fb55a5d5c5c1466576fbef9a6cd06f67a7878389148ecdab5aba5a5a5bbbb3b1a8d9a8589ba3773db73fc861096f56babaa9a4c26eb9619ad3029c462b1e9e9699bdb5055f5d9b367d6bc2d9bcdf2c9042164fffefda619abababc2d4bea5a58557a469437f7f3f5bbed93efae38f3f3c08cbc986d6d656a7dd8b81818181810127f36a4f572c163ffffcf3ba567df0c107c160b0ee65a81142e1f0f070b95cb6b6504a59e9c87d04c118c7e371db38e845066d6de153318450a552191c1c34dbd7d6d6844fa2b6d6bbbbbb85e1f8e9d3a775ede7595f5f17daa0ebbaf0fa2dba1c8cb1af1eaaaaba9f65e785a5ebfad8d898f591300ff4d34f3f6dea66298a22dc12999f9faff196d1d7d7679dab5c2e0ba7e6179856d85e218fb0125b1761da5ed786c661e7854508e9eded8d442256ff84317ef0e0c1c3870fdd8f4329edefefe79ddcfafa7a369b65afb3d96ca954e23b1e3e7cd8da5158ca47f516e42c45e3db793fea06a7f298fb4d959da521aca4948e8f8ffbfd7eeb83d134ede6cd9b4ef7970763dcd5d54508b13d5d9fcf677aa9f9f9795b92c4f66ec3e1309f0b7b40d8d1292df330d4bf88861016c65855d577df7dd7262342c8a54b97dc8fa328cae0e0203f78269361af9f3c79c23fb0575e7985af3f09c7e7cbaa568ac5a2b05db8e55c17271b9cbce916a1946ed46353bb9f0db12a4408514a9b9b9be3f1f8ececac794f31c66b6b6bc27d3d27e2f1f8efbfff6e6bfcfbefbfcd17b6a11445e15752cc77f212e4c3a86d16a19b696f6f7763b98d402020b4a146605d5b5b7bfcf8f1810307dcd4d06d0483c18f3ffe78d3563ad328c26277f0f8f1e34b4b4bcf9f3fb7b6271209f777caeff7070201db928a10f2f0e1437e4b98521a0c06f974d8300ce14f339fcf23e79a502a9512b6777676ba34de8d0dacda29b4616e6e2e994c5ebf7ebda3a3a3b7b7371c0eb7b7b7b34a58dd3ad6b61f826814613158b275f1e245eb66fea6d255160defdcb963bd8f8aa2cccecef24311425e7ef96561b553783c359fcfe77239b69f68331b21f4e79f7f0a4dededed756fbf89aeeb8aa2f036b02230bf1f85319e9d9d5514251008e472b94422c1d2c75028343434b47fff7e0f366c8586c8b14c30c6baae4f4c4c6c6590c1c1419b87c318e7f3f97c3e6f7b1e7ebfffd0a1434e83f08daaaa4e4e4ef29e8f1dc41306299fcfd7d4d4e4cd1f0c0d0d09db6fdebc696bc1183f7efc18590487315614c5e7f33d7ffedc5bbd638b3496b0104294d2cecece6834ea7e3dc8c3471f611ceceaea721ae1e8d1a3c2bdbf42a170fdfa759b409f3c7972e5ca15e1de22db29f2b6c41b1818e06d5014e5fefdfbb6aa6f2e97e3ad62a8aa7af4e8d1ba73fd6f6ee9586131686c6cec9b6fbef1b67bca0a5a8b8b8bd675beb0da7ee4c811a7e4a3a9a969cf9e3d994c467818e6cb2fbfecebeb6b6b6b2b97cba9542a954af9fd7ee13104b6afe78d402010894452a9944d5e18e3c9c9c96834dad3d3a3284a269379f0e081300da594eedbb7cf8d688ac5e2679f7de6c6aa1a67f3ad349cb0d00b6d8d8f8f5fbe7cd9c30207631c8bc5ea568f745dafe1b128a5efbcf3ce850b17f82a3cc6b85aaddebb77cf6c1156c30921478e1cf1566b306d181f1fffeaabaff8ed1d4dd3d2e9743a9d666f6bd426464747dd6ce763d1517a212e53de860b850c8c717b7bfbebafbfee79b5128fc76b5f70ecd8b1ba834c4c4c78ab9b23847a7a7a464646b6beda9a9898a85d3f73829dc50806833b526b6d5061218428a57d7d7d1d1d1d1e9e0d3bec50234bab56abd168b4c6c8ec61b4b5b59d3d7b7653dbe16cf043870e9d3a75cafdc99f1a363437379f3d7b96df51a86bc38913270e1c38f0fffec7143c2c20befdf6dbdea2617b7bbb6d8fc88452dad2d2d2dcdcece6a9efdebdfba38f3e628b89daeb094a69a552090683274f9e646706b7cb558442a10f3ffc90fd3d921b1b745d3f73e60cdb39dda9ada1edc9b10cc3e01fbff0909aaeebb6eca74632c4aa59a74f9ffee1871faca1dde50effe8e8e89d3b77841f9d3c79d2cd080c5555c7c6c6aad5ead4d4542a9532ff7e81a9962dec5555d5757d6868c83c5feaf444fd7ebff04853eda4d0e7f39d3e7dba542addba752b9d4e572a15e6c3ccb9d8b698dfef3f7efc383b5b26b4811574dc7ff7cddaf9df89b6ee2a6bfc2c6c1f79f801397571534adee2054ebd98eb32ef1b7e01fb69fd034e82b70159f4fdcfd85097edf9137b00b0d1b83916f0af06840548018405480184054801840548018405480184054801840548018405480184054801840548018405480184054801840548018405480184054801840548018405480184054801840548018405480184054801840548018405480184054801840548018405480184054801840548018405480184054801840548018405480184054801840548e13f4962dbdeba40af160000000049454e44ae426082";
   return logo;
}


QString add_longitud_auxiliar(QStringList cuentas, QString auxiliar, int ampliar)
{
    QString ceros;
    for (int veces=0; veces<ampliar; veces++) ceros.append("0");
    for (int veces=auxiliar.length()-1; veces>=0; veces--)
       {
        if (cuentas.contains(auxiliar.left(veces+1)))
          {
             QString sustituida;
             sustituida=auxiliar;
             sustituida.insert(veces,ceros);
             return sustituida;
          }
       }
    return auxiliar+ceros;
}


bool floats_2d_iguales(double num1, double num2)
{
  if (num1-num2>-0.0001 && num1-num2<0.0001) return true;
  return false;
}


bool nif_ue_correcto(QString nif_iva, QString &nombre, QString &direccion, bool &hayerror)
{
    bool correcto=false;

    checkVatBindingProxy vatnum;
    _ns2__checkVat datos;
    _ns2__checkVatResponse respuesta;

    datos.countryCode=nif_iva.left(2).toLocal8Bit().constData();
    datos.vatNumber=nif_iva.mid(2).toLocal8Bit().constData();

    hayerror=false;

    if (vatnum.checkVat(&datos,respuesta)==SOAP_OK)
      {
        if (respuesta.valid)
         {
          nombre=QString::fromUtf8(respuesta.name->c_str());
          direccion=QString::fromUtf8(respuesta.address->c_str());
          // std::cout << "CORRECTO\n" ;
          // std::cout << respuesta.name->c_str() << std::endl;
          // std::cout << respuesta.address->c_str() << std::endl;
          correcto=true;
         }
           else correcto=false;// std::cout << "INCORRECTO" << std::endl;
      }
    else
        {
         correcto=false;
         hayerror=true;
        }

    vatnum.destroy(); // same as: soap_destroy(calc.soap); soap_end(calc.soap);

    return correcto;
}
