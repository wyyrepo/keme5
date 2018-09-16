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

#include "factura.h"
#include "funciones.h"
#include "basedatos.h"
#include "buscasubcuenta.h"
#include "aux_express.h"
#include "datos_accesorios.h"
#include "edita_ref.h"
#include "buscaref.h"
#include <QMessageBox>



factura::factura() : QDialog() {
    ui.setupUi(this);

coma=haycomadecimal();
decimales=haydecimales();

ui.fechadateEdit->setDate(QDate::currentDate());
ui.fechaopdateEdit->setDate(QDate::currentDate());
ui.doccomboBox->addItems(basedatos::instancia()->listacoddocs());
ret_cambiado();
QString cuenta; double tipo;
basedatos::instancia()->carga_irpf(&cuenta,&tipo);
ui.tipo_ret_lineEdit->setText(formateanumero(tipo,coma,true));
connect(ui.doccomboBox, SIGNAL(currentIndexChanged(int)), this,
          SLOT (actudoc()));
connect(ui.tableWidget,SIGNAL( cellChanged ( int , int )),this,
       SLOT(contenidocambiado(int,int )));
connect(ui.cta_cli_lineEdit,SIGNAL(textChanged(QString)),this,SLOT(ctabasecambiada()));
connect(ui.cta_cli_lineEdit,SIGNAL(editingFinished()),this,SLOT(ctabasefinedicion()));
connect(ui.ctapushButton,SIGNAL(clicked()),this,SLOT(buscactacli()));
connect(ui.edctapushButton,SIGNAL(clicked()),SLOT(datoscuenta()));
connect(ui.ret_checkBox,SIGNAL(stateChanged(int)),SLOT(ret_cambiado()));
connect(ui.aceptarpushButton,SIGNAL(clicked()),this,SLOT(aceptar()));
connect(ui.tableWidget,SIGNAL(  currentCellChanged ( int , int, int, int )),this,
       SLOT(posceldacambiada ( int, int, int, int  )));

connect(ui.BotonBorraLinea,SIGNAL(  clicked()),this,SLOT(borralinea( void )));
connect(ui.arribapushButton,SIGNAL(clicked()),SLOT(muevearriba()));
connect(ui.abajopushButton ,SIGNAL(clicked()),SLOT(mueveabajo()));

connect(ui.pdfpushButton ,SIGNAL(clicked()),SLOT(gen_pdf()));

tipo_operacion=0;

actudoc();

// construir una lista con tipo de iva, re, baseimponible
QSqlQuery query =basedatos::instancia()->selectTodoTiposivanoclave("");
if (query.isActive())
    while (query.next())
       {
         codigoiva << query.value(0).toString();
         tipoiva << query.value(1).toDouble()/100;
         tipore << query.value(2).toDouble()/100;
         baseimponible << 0;
       }
 modo_consulta=false;
 ui.tableWidget->setEditTriggers(QAbstractItemView::AllEditTriggers);
 ui.tableWidget->setColumnWidth(0,90);
 ui.tableWidget->setColumnWidth(1,250);
 ui.tableWidget->setColumnWidth(2,90);
 ui.tableWidget->setColumnWidth(3,100);
 ui.tableWidget->setColumnWidth(4,90);
 ui.tableWidget->setColumnWidth(6,90);
 predefinida=false;
}

QString factura::ltipoiva(QString codigo)
{
  int pos=codigoiva.indexOf(codigo);
  double valor=0;
  if (pos>=0)
      valor=tipoiva.at(pos);
  valor=valor*100;
  return formateanumero(valor,coma,decimales);
}

QString factura::ltipore(QString codigo)
{
    int pos=codigoiva.indexOf(codigo);
    double valor=0;
    if (pos>=0)
        valor=tipore.at(pos);
    valor=valor*100;
    return formateanumero(valor,coma,decimales);

}

void factura::fijatipoiva(QString codigo, double tporuno)
{
    int pos=codigoiva.indexOf(codigo);
    if (pos>=0)
        tipoiva.replace(pos,tporuno);
      else
          {
            codigoiva << codigo;
            tipoiva << tporuno;
            tipore << 0;
            baseimponible << 0;
          }

}


void factura::fijatipore(QString codigo, double tporuno)
{
    int pos=codigoiva.indexOf(codigo);
    if (pos>=0)
        tipore.replace(pos,tporuno);

}



void factura::actudoc()
{
 // actualizar serie
 // actualizar nombre documento
 QString logo;
 QString documento, cantidad, referencia, descripref, precio, totallin, bi;
 QString tipoiva, tipore, cuota, cuotare, totalfac, notas, venci, notastex;
 QString cif_empresa, cif_cliente, msnumero, msfecha, mscliente, msdescuento;
 bool rectificativo;
 QString totallineas;
 QString lineas_doc;
 QString nombre_emisor;
 QString domicilio_emisor;
 QString cp_emisor;
 QString poblacion_emisor;
 QString provincia_emisor;
 QString pais_emisor;
 QString cif_emisor;
 QString id_registral;


 basedatos::instancia()->carga_tipo_doc(ui.doccomboBox->currentText(),
                                   &descripfact,
                                   &seriefact,
                                   &pie1,
                                   &pie2,
                                   &moneda,
                                   &codigo_moneda,
                                   &contabilizable,
                                   &rectificativo,
                                   &tipo_operacion,
                                   &documento, &cantidad, &referencia, &descripref,
                                   &precio, &totallin, &bi,
                                   &tipoiva, &tipore, &cuota, &cuotare, &totalfac,
                                   &notas, &venci, &notastex, &cif_empresa, &cif_cliente,
                                   &msnumero, &msfecha, &mscliente, &msdescuento,
                                   &totallineas, &lineas_doc, &nombre_emisor,
                                   &domicilio_emisor, &cp_emisor, &poblacion_emisor,
                                   &provincia_emisor, &pais_emisor, &cif_emisor,
                                   &id_registral,
                                   &logo);
 ui.serielineEdit->setText(seriefact);
 ui.documentolabel->setText(descripfact);
 ui.pie1lineEdit->setText(pie1);
 ui.pie2lineEdit->setText(pie2);
 ui.textEdit->setText(notastex);

 QPixmap foto;
 if (logo.length()>0)
       {
        QByteArray byteshexa;
        byteshexa.append ( logo );
        QByteArray bytes;
        bytes=bytes.fromHex ( byteshexa );
        foto.loadFromData ( bytes, "PNG");
       }
 ui.logo_label->setPixmap(foto);

}




void factura::contenidocambiado(int fila,int columna)
{

 if (ui.tableWidget->currentColumn()!=columna || ui.tableWidget->currentRow()!=fila) return;
 QMessageBox msgBox;
 QPushButton *aceptarButton = msgBox.addButton(tr("Sí"), QMessageBox::ActionRole);
 msgBox.addButton(tr("No"), QMessageBox::ActionRole);
 msgBox.setWindowTitle(tr("ADVERTENCIA"));
 msgBox.setIcon(QMessageBox::Warning);
// Referencia
// Descripción
// Cantidad
// precio
 // total
 // iva
if (columna==0) {
    //
    for (int veces=1;veces<ui.tableWidget->columnCount();veces++)
         if (ui.tableWidget->item(fila,veces)==0)
             {
              QTableWidgetItem *newItem = new QTableWidgetItem("");
              if (veces==2 || veces==3 || veces==4 || veces==5 || veces==7 || veces==8)
                  newItem->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
              ui.tableWidget->setItem(fila,veces,newItem);
             }
             else if (veces==2)
                   ui.tableWidget->item(fila,veces)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

    //
    QString descrip, tipoiva, tipoivare, cuenta;
    double pvp;
    QString codigo=ui.tableWidget->item(fila,columna)->text();
    if (codigo.length()==1)
      {
        buscaref *b = new buscaref(codigo);
        int cod=b->exec();
        QString cadena2=b->seleccioncuenta();
        if (cod==QDialog::Accepted )
           ui.tableWidget->item(fila,columna)->setText(cadena2);
        codigo=cadena2;
      }
    if (!basedatos::instancia()->existecodigoref (codigo,&descrip) && !codigo.isEmpty())
      {
        QString TextoAviso = tr("El código suministrado no existe \n"
                     "¿ desea añadir la referencia ?");
        msgBox.setText(TextoAviso);
        msgBox.exec();
        if (msgBox.clickedButton() == aceptarButton)
          {
            basedatos::instancia()->insertref(codigo,QString());
            edita_ref *e = new edita_ref();
            e->pasacodigo(codigo);
            e->exec();
            delete(e);
          }
      }
    if (basedatos::instancia()->existecodigoref (codigo,&descrip))
       {

        basedatos::instancia()->carga_ref(codigo,
                                   &descrip,
                                   &tipoiva,
                                   &tipoivare,
                                   &cuenta,
                                   &pvp);
        // Referencia
        // Descripción
        // Cantidad
        // precio
         // total
         // iva
        ui.tableWidget->item(fila,1)->setText(descrip);
        ui.tableWidget->item(fila,2)->setText("1");
        QString cadpvp=formateanumero(pvp,coma,decimales);
        ui.tableWidget->item(fila,3)->setText(cadpvp);
        //ui.tableWidget->item(fila,4)->setText("0");
        if (tipo_operacion>1) tipoiva=tipoivaexento();
        QString codigoiva=ui.recheckBox->isChecked() ? tipoivare : tipoiva;
        ui.tableWidget->item(fila,6)->setText(codigoiva);
        ui.tableWidget->item(fila,7)->setText(ltipoiva(codigoiva));
        ui.tableWidget->item(fila,8)->setText(ltipore(codigoiva));
        double total;
        total=convapunto(ui.tableWidget->item(fila,2)->text()).toDouble() *
              convapunto(ui.tableWidget->item(fila,3)->text()).toDouble();
        ui.tableWidget->item(fila,5)->setText(formateanumero(total,coma,decimales));

       }
       else ui.tableWidget->item(fila,0)->setText("");
   }

if (columna==2 || columna==3 || columna==4 || columna==5)
  {    
    if (ui.tableWidget->item(fila,0)==NULL)
     { ui.tableWidget->item(fila,columna)->setText(""); return; }

    if (ui.tableWidget->item(fila,0)->text().isEmpty())
     { ui.tableWidget->item(fila,columna)->setText(""); return; }
    if (
        ui.tableWidget->item(fila,1)==NULL ||
        ui.tableWidget->item(fila,2)==NULL ||
        ui.tableWidget->item(fila,3)==NULL ||
        ui.tableWidget->item(fila,4)==NULL ||
        ui.tableWidget->item(fila,5)==NULL  ) return;
   if (ui.tableWidget->item(fila,0)->text().isEmpty())
    { ui.tableWidget->item(fila,columna)->setText(""); return; }
   float numero=convapunto(ui.tableWidget->item(fila,columna)->text()).toDouble();
   if (columna==2 && coma)
       ui.tableWidget->item(fila,2)->setText(coma ?
                                             convacoma(ui.tableWidget->item(fila,columna)->text()) :
                                             convapunto(ui.tableWidget->item(fila,columna)->text()));
   if (columna==3)
       ui.tableWidget->item(fila,3)->setText(formateanumero(numero,coma,decimales));
   if (columna==4)
       ui.tableWidget->item(fila,4)->setText(formateanumero(numero,coma,decimales));
    double total;
    total=convapunto(ui.tableWidget->item(fila,2)->text()).toDouble() *
          convapunto(ui.tableWidget->item(fila,3)->text()).toDouble() *
          (1-convapunto(ui.tableWidget->item(fila,4)->text()).toDouble()/100);
    ui.tableWidget->item(fila,5)->setText(formateanumero(total,coma,decimales));
  }

if (columna==6 || columna==7 || columna==8)
 {
  // No admitimos cambios en tipo de IVA
    if (ui.tableWidget->item(fila,0)==NULL) return;
    QString descrip, tipoiva, tipoivare, cuenta;
    double pvp;
    QString codigo=ui.tableWidget->item(fila,0)->text();
    if (basedatos::instancia()->existecodigoref (codigo,&descrip))
       {

        basedatos::instancia()->carga_ref(codigo,
                                   &descrip,
                                   &tipoiva,
                                   &tipoivare,
                                   &cuenta,
                                   &pvp);
        // Referencia
        // Descripción
        // Cantidad
        // precio
         // total
         // iva
        if (tipo_operacion>1) tipoiva=tipoivaexento();
        QString codigoiva=ui.recheckBox->isChecked() ? tipoivare : tipoiva;
        ui.tableWidget->item(fila,6)->setText(codigoiva);
        ui.tableWidget->item(fila,7)->setText(ltipoiva(codigoiva));
        ui.tableWidget->item(fila,8)->setText(ltipore(codigoiva));
       }
    else ui.tableWidget->item(fila,6)->setText("");

 }


actualizatotales();

// QMessageBox::warning( this, tr("facturas"),tr("PASAMOS DE totales"));

if (ui.tableWidget->currentColumn()==0)
   {
    if (ui.tableWidget->item(ui.tableWidget->currentRow(),0)==NULL) return;
    if (ui.tableWidget->item(ui.tableWidget->currentRow(),2)==NULL) return;
    ui.tableWidget->setCurrentCell(ui.tableWidget->currentRow(),2);
    return;
   }

if (ui.tableWidget->currentRow()>ui.tableWidget->rowCount()-2) return;

ui.tableWidget->disconnect(SIGNAL(currentCellChanged(int,int,int,int)));
ui.tableWidget->setCurrentCell(ui.tableWidget->currentRow()+1,0);
connect(ui.tableWidget,SIGNAL(  currentCellChanged ( int , int, int, int )),this,
       SLOT(posceldacambiada ( int, int, int, int  )));

return;

}


void factura::actualizatotales()
{
    double base, cuotaIVA, cuotaRE;
    base=0;
    cuotaIVA=0;
    cuotaRE=0;
    for (int veces=0; veces<baseimponible.size(); veces++) baseimponible[veces]=0;
    for (int veces=0;veces<ui.tableWidget->rowCount();veces++)
       {
        if (ui.tableWidget->item(veces,0)==NULL) continue;
        if (ui.tableWidget->item(veces,5)==NULL) continue;
        if (ui.tableWidget->item(veces,6)==NULL) continue;
        // actualizar listas con baseimponible
        bool encontrado=false;
        for (int veces2=0; veces2<baseimponible.size(); veces2++)
           {
            if (codigoiva.at(veces2)==ui.tableWidget->item(veces,6)->text())
              {
                baseimponible[veces2]=baseimponible.at(veces2)+
                                      convapunto(ui.tableWidget->item(veces,5)->text()).toDouble();
                base+=convapunto(ui.tableWidget->item(veces,5)->text()).toDouble();
                cuotaIVA+=convapunto(ui.tableWidget->item(veces,5)->text()).toDouble()*tipoiva.at(veces2);
                cuotaRE+=convapunto(ui.tableWidget->item(veces,5)->text()).toDouble()*tipore.at(veces2);
                encontrado=true;
              }
           }
        // si no encontramos, suponemos IVA exento
        if (!encontrado) base+=convapunto(ui.tableWidget->item(veces,5)->text()).toDouble();
       }
    ui.bilineEdit->setText(formateanumerosep(base,coma,decimales));
    ui.cuotaIVAlineEdit->setText(formateanumerosep(cuotaIVA,coma,decimales));
    ui.cuotaRElineEdit->setText(formateanumerosep(cuotaRE,coma,decimales));
    ui.totallineEdit->setText(formateanumerosep(base+cuotaIVA+cuotaRE,coma,decimales));
    double tipo=convapunto(ui.tipo_ret_lineEdit->text()).toDouble();
    if (ui.ret_checkBox->isChecked())
      {
       ui.ret_lineEdit->setText(formateanumero(tipo*base/100,coma,decimales));
       ui.totallineEdit->setText(formateanumerosep(base+cuotaIVA+cuotaRE-tipo*base/100,coma,decimales));
      }
}


void factura::ctabasecambiada()
{
 QMessageBox msgBox;
 QPushButton *aceptarButton = msgBox.addButton(tr("Sí"), QMessageBox::ActionRole);
 msgBox.addButton(tr("No"), QMessageBox::ActionRole);
 msgBox.setWindowTitle(tr("ADVERTENCIA"));
 msgBox.setIcon(QMessageBox::Warning);

    QString cadena;
    if (ui.cta_cli_lineEdit->text().length()==anchocuentas() && !cod_longitud_variable()) {
        if (!existecodigoplan(ui.cta_cli_lineEdit->text(),&cadena))
          {
         // preguntar si se desea añadir la cuenta, si no o si se cancela lo anterior dejar en blanco
            QString TextoAviso = tr("Esa cuenta no existe, ¿ Desea añadirla ?");
            msgBox.setText(TextoAviso);
            msgBox.exec();
            if (msgBox.clickedButton() == aceptarButton)
                 {
                   aux_express *lasubcuenta2=new aux_express();
                   lasubcuenta2->pasacodigo(ui.cta_cli_lineEdit->text());
                   lasubcuenta2->exec();
                   delete(lasubcuenta2);
                   if (!existecodigoplan(ui.cta_cli_lineEdit->text(),&cadena))
                        ui.cta_cli_lineEdit->setText("");
                 }
                  else
                    ui.cta_cli_lineEdit->setText("");
           }
        }
   if (esauxiliar(ui.cta_cli_lineEdit->text()))
       ui.clientelabel->setText(descripcioncuenta(ui.cta_cli_lineEdit->text()));
     else ui.clientelabel->setText("");
}

void factura::ctabasefinedicion()
{
  QString cadena;
  ui.cta_cli_lineEdit->setText(expandepunto(ui.cta_cli_lineEdit->text(),anchocuentas()));
 QMessageBox msgBox;
 QPushButton *aceptarButton = msgBox.addButton(tr("Sí"), QMessageBox::ActionRole);
 msgBox.addButton(tr("No"), QMessageBox::ActionRole);
 msgBox.setWindowTitle(tr("ADVERTENCIA"));
 msgBox.setIcon(QMessageBox::Warning);
  if (ui.cta_cli_lineEdit->text().length()>3 && cod_longitud_variable()) {
        if (!existecodigoplan(ui.cta_cli_lineEdit->text(),&cadena))
          {
         // preguntar si se desea añadir la cuenta, si no o si se cancela lo anterior dejar en blanco
            QString TextoAviso = tr("Esa cuenta no existe, ¿ Desea añadirla ?");
            msgBox.setText(TextoAviso);
            msgBox.exec();
            if (msgBox.clickedButton() == aceptarButton)
                 {
                   aux_express *lasubcuenta2=new aux_express();
                   lasubcuenta2->pasacodigo(ui.cta_cli_lineEdit->text());
                   lasubcuenta2->exec();
                   delete(lasubcuenta2);
                   if (!esauxiliar(ui.cta_cli_lineEdit->text()))
                        ui.cta_cli_lineEdit->setText("");
                 }
                  else
                    ui.cta_cli_lineEdit->setText("");
           }
        if (esauxiliar(ui.cta_cli_lineEdit->text()))
          ui.clientelabel->setText(descripcioncuenta(ui.cta_cli_lineEdit->text()));
         else ui.clientelabel->setText("");
        }

}


void factura::buscactacli()
{
    QString cadena2;
    buscasubcuenta *labusqueda=new buscasubcuenta(ui.cta_cli_lineEdit->text());
    labusqueda->exec();
    cadena2=labusqueda->seleccioncuenta();
    if (esauxiliar(cadena2)) ui.cta_cli_lineEdit->setText(cadena2);
       else ui.cta_cli_lineEdit->setText("");
    delete(labusqueda);
}


void factura::datoscuenta()
{
    QString cuenta=ui.cta_cli_lineEdit->text();
    if (cuenta.isEmpty()) return;
    if (!existesubcuenta(cuenta)) return;
    datos_accesorios *d = new datos_accesorios();
    d->cargacodigo( cuenta );
    d->exec();
    delete(d);
}


void factura::ret_cambiado()
{
    if (ui.ret_checkBox->isChecked())
       {
        ui.ret_label->setVisible(true);
        ui.tipo_ret_lineEdit->setVisible(true);
        ui.ret_lineEdit->setVisible(true);
        ui.porcent_label->setVisible(true);
       }
      else
          {
           ui.ret_label->setVisible(false);
           ui.tipo_ret_lineEdit->setVisible(false);
           ui.ret_lineEdit->setVisible(false);
           ui.porcent_label->setVisible(false);
          }
    actualizatotales();
    /*
 double base=convapunto(ui.bilineEdit->text()).toDouble();
 double cuotaIVA=convapunto(ui.cuotaIVAlineEdit->text()).toDouble();
 double cuotaRE=convapunto(ui.cuotaRElineEdit->text()).toDouble();
 if (ui.ret_checkBox->isChecked())
    {
     double tipo=convapunto(ui.tipo_ret_lineEdit->text()).toDouble();
     ui.ret_lineEdit->setText(formateanumero(tipo*base/100,coma,decimales));
     ui.totallineEdit->setText(formateanumerosep(base+cuotaIVA+cuotaRE-tipo*base/100,coma,decimales));
     ui.ret_label->setVisible(true);
     ui.tipo_ret_lineEdit->setVisible(true);
     ui.ret_lineEdit->setVisible(true);
     ui.porcent_label->setVisible(true);
    }
   else
       {
        ui.totallineEdit->setText(formateanumerosep(base+cuotaIVA+cuotaRE,coma,decimales));
        ui.ret_label->setVisible(false);
        ui.tipo_ret_lineEdit->setVisible(false);
        ui.ret_lineEdit->setVisible(false);
        ui.porcent_label->setVisible(false);
       }
*/
}

void factura::latexdoc(QString serie, QString numero)
{

    QString fichero=latex_doc(  serie,numero);
    if (fichero.isEmpty())
    {
     QMessageBox::warning( this, tr("Edición de documentos"),
                           tr("ERROR: No se ha podido generar Latex"));
     return;
    }

    if (!genera_pdf_latex(fichero))
       {
        QMessageBox::information( this, tr("FACTURA EN PDF"),
                                  tr("ERROR: no ha sido posible generar el archivo PDF"));
        return;
       }
    QString archivopdf=fichero;
    archivopdf.truncate(archivopdf.length()-4);
    archivopdf.append(".pdf");
    if (!ejecuta(aplicacionabrirfich(extensionfich(archivopdf)),archivopdf))
          QMessageBox::warning( this, tr("FACTURA EN PDF"),
                              tr("No se puede abrir ")+archivopdf+tr(" con ")+
                              aplicacionabrirfich(extensionfich(archivopdf)) + "\n" +
                              tr("Verifique el diálogo de preferencias"));

}


void factura::terminar(bool impri)
{
    if (predefinida)
       {
        guarda_predef();
        accept();
        return;
       }

    if (modo_consulta)
     {
       if (impri) latexdoc(ui.serielineEdit->text(), ui.facturalineEdit->text());
       accept();
       return;
     }
    // si el campo serielineEdit está vacío, mensaje y no permitir la grabación

    // Comenzamos transacción: tablas series_fact, facturas, detalle_fact (añadir)
    QSqlDatabase contabilidad=QSqlDatabase::database();
    contabilidad.transaction();

    basedatos::instancia()->bloquea_para_facturas();

   // Está el campo facturalineEdit vacío ?
   //
   //   Sí --->  aumentamos campo de número (tabla series_fact) y asignamos a facturalineEdit;
    //           incrementamos valor campo proxnum, para código de serie (codigo) en series_fact

   if (ui.facturalineEdit->text().isEmpty())
    {
     qlonglong numero=basedatos::instancia()->proxnum_serie(ui.serielineEdit->text());
     QString cadnum; cadnum.setNum(numero);
     ui.facturalineEdit->setText(cadnum);
    }

   // comprobamos si existe cabecera de factura
    long clave_cabecera;
    if (basedatos::instancia()->existe_fact(ui.serielineEdit->text(), ui.facturalineEdit->text()))
       {
        // actualizamos
        clave_cabecera=basedatos::instancia()->modificacabecerafactura(
                                            ui.serielineEdit->text(),
                                            ui.facturalineEdit->text(),
                                            ui.cta_cli_lineEdit->text(),
                                            ui.fechadateEdit->date(),
                                            ui.fechaopdateEdit->date(),
                                            false, // contabilizado
                                            contabilizable,
                                            ui.ret_checkBox->isChecked(),
                                            ui.recheckBox->isChecked(),
                                            convapunto(ui.tipo_ret_lineEdit->text()),
                                            convapunto(ui.ret_lineEdit->text()),
                                            ui.doccomboBox->currentText(),
                                            ui.textEdit->toPlainText(),
                                            ui.pie1lineEdit->text(),
                                            ui.pie2lineEdit->text(),
                                            "0"
                                            );
        // borramos registros de detalle
        basedatos::instancia()->borralineas_doc(clave_cabecera);
       }
      else
         {
          // insertamos
          clave_cabecera=basedatos::instancia()->nuevacabecerafactura(
                  ui.serielineEdit->text(),
                  ui.facturalineEdit->text(),
                  ui.cta_cli_lineEdit->text(),
                  ui.fechadateEdit->date(),
                  ui.fechaopdateEdit->date(),
                  false, // contabilizado
                  contabilizable,
                  ui.ret_checkBox->isChecked(),
                  ui.recheckBox->isChecked(),
                  convapunto(ui.tipo_ret_lineEdit->text()),
                  convapunto(ui.ret_lineEdit->text()),
                  ui.doccomboBox->currentText(),
                  ui.textEdit->toPlainText(),
                  ui.pie1lineEdit->text(),
                  ui.pie2lineEdit->text(),
                  "0"
                     );

         }
    // añadimos registros de detalle
    int ultimo=ultimafila();
    for (int veces=0; veces<ultimo; veces++)
       {
        if (ui.tableWidget->item(veces,0)==NULL) continue;
        QString tipo,re;
        cadvalorestipoiva(ui.tableWidget->item(veces,6)->text(), &tipo, &re);
        basedatos::instancia()->insert_lin_doc(clave_cabecera,
                                       ui.tableWidget->item(veces,0)->text(),   // codigo
                                       ui.tableWidget->item(veces,1)->text(),   // descrip
                                       convapunto(ui.tableWidget->item(veces,2)->text()),   // cantidad
                                       convapunto(ui.tableWidget->item(veces,3)->text()),   // precio
                                       ui.tableWidget->item(veces,6)->text(), // clave_iva
                                       tipo, // tipo_iva
                                       re, // recargo equivalencia
                                       convapunto(ui.tableWidget->item(veces,4)->text())); // dto.
       }
    // terminamos
    basedatos::instancia()->desbloquea_y_commit();
    if (impri) latexdoc(ui.serielineEdit->text(), ui.facturalineEdit->text());
    accept();

}

void factura::aceptar()
{
    terminar(false); // no pdf
}


void factura::gen_pdf()
{
    terminar(true); // pdf
}


void factura::pasa_cabecera_doc(QString serie,
                       QString numero,
                       QString cuenta,
                       QDate fecha_fac,
                       QDate fecha_op,
                       bool contabilizable,
                       bool con_ret,
                       bool re,
                       QString tipo_ret,
                       QString retencion,
                       QString tipo_doc,
                       QString notas,
                       QString pie1,
                       QString pie2
                       )
{
    ui.serielineEdit->setText(serie);
    ui.facturalineEdit->setText(numero);
    ui.cta_cli_lineEdit->setText(cuenta);
    ui.fechadateEdit->setDate(fecha_fac);
    ui.fechaopdateEdit->setDate(fecha_op);
    contabilizable=contabilizable;
    ui.ret_checkBox->setChecked(con_ret);
    ui.recheckBox->setChecked(re);
    if (tipo_ret.toFloat()<0.0001 && tipo_ret.toFloat()>-0.001)
       {
        double qtipo;
        basedatos::instancia()->carga_irpf(&cuenta,&qtipo);
        ui.tipo_ret_lineEdit->setText(formateanumero(qtipo,coma,true));
        ui.ret_lineEdit->setText(coma ? convacoma(0) : 0);
       }
      else
     {
      ui.tipo_ret_lineEdit->setText(coma ? convacoma(tipo_ret) : tipo_ret);
      ui.ret_lineEdit->setText(coma ? convacoma(retencion) : retencion);
     }
    for (int veces=0; veces<ui.doccomboBox->count(); veces++)
        {
         if (ui.doccomboBox->itemText(veces)==tipo_doc)
            {
             ui.doccomboBox->setCurrentIndex(veces);
             break;
            }
        }
    ui.textEdit->setText(notas);
    ui.pie1lineEdit->setText(pie1);
    ui.pie2lineEdit->setText(pie2);

    ui.doccomboBox->setEnabled(false);

    // averiguamos clave
    int clave=basedatos::instancia()->clave_doc_fac(serie,numero);

    QSqlQuery q;
    q = basedatos::instancia()->select_lin_doc(clave);
    int veces=0;
    if (q.isActive())
       while (q.next())
        {
        for (int veces2=0;veces2<ui.tableWidget->columnCount();veces2++)
             if (ui.tableWidget->item(veces,veces2)==0)
                 {
                  QTableWidgetItem *newItem = new QTableWidgetItem("");
                  if (veces2==2 || veces2==3 || veces2==4 || veces2==5)
                      newItem->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
                  ui.tableWidget->setItem(veces,veces2,newItem);
                 }
        // clave, codigo, descripcion, cantidad, precio, clave_iva, tipo_iva, tipo_re, dto
        ui.tableWidget->item(veces,0)->setText(q.value(1).toString());
        ui.tableWidget->item(veces,1)->setText(q.value(2).toString());
        ui.tableWidget->item(veces,2)->setText(coma ? convacoma(q.value(3).toString())
            : q.value(3).toString());
        double cantidad=q.value(3).toDouble();
        double precio=q.value(4).toDouble();
        double totalneto=cantidad*precio*(1-q.value(8).toDouble()/100);
        ui.tableWidget->item(veces,3)->setText(formateanumero(q.value(4).toDouble(),coma,decimales));
        ui.tableWidget->item(veces,4)->setText(formateanumero(q.value(8).toDouble(),coma,decimales));
        ui.tableWidget->item(veces,5)->setText(formateanumero(totalneto,coma,decimales));
        ui.tableWidget->item(veces,6)->setText(q.value(5).toString());
        // tipo iva y re lugares 6 y 7
        ui.tableWidget->item(veces,7)->setText(formateanumero(q.value(6).toDouble(),
                                                              coma,decimales));
       //  QMessageBox::warning( this, tr("facturas"),formateanumero(q.value(6).toDouble(),
       //                                                           coma,decimales));

        ui.tableWidget->item(veces,8)->setText(formateanumero(q.value(7).toDouble(),
                                                              coma,decimales));
        // actualizar listas con tipos de iva y re
        fijatipoiva(q.value(5).toString(),q.value(6).toDouble()/100);
        fijatipore(q.value(5).toString(),q.value(7).toDouble()/100);
        veces++;

        }
    actualizatotales();
}


void factura::solo_cabecera_doc(QString serie,
                       QString numero,
                       QString cuenta,
                       QDate fecha_fac,
                       QDate fecha_op,
                       bool contabilizable,
                       bool con_ret,
                       bool re,
                       QString tipo_ret,
                       QString retencion,
                       QString tipo_doc
                       )
{
    ui.serielineEdit->setText(serie);
    ui.facturalineEdit->setText(numero);
    ui.cta_cli_lineEdit->setText(cuenta);
    ui.fechadateEdit->setDate(fecha_fac);
    ui.fechaopdateEdit->setDate(fecha_op);
    contabilizable=contabilizable;
    ui.ret_checkBox->setChecked(con_ret);
    ui.recheckBox->setChecked(re);
    ui.tipo_ret_lineEdit->setText(coma ? convacoma(tipo_ret) : tipo_ret);
    ui.ret_lineEdit->setText(coma ? convacoma(retencion) : retencion);
    for (int veces=0; veces<ui.doccomboBox->count(); veces++)
        {
         if (ui.doccomboBox->itemText(veces)==tipo_doc)
            {
             ui.doccomboBox->setCurrentIndex(veces);
             break;
            }
        }

    ui.doccomboBox->setEnabled(false);
    actudoc();
}


int factura::ultimafila()
{
  int veces=0;
  while (veces< ui.tableWidget->rowCount ())
    {
      if (ui.tableWidget->item(veces,0)==0) break;
      if (ui.tableWidget->item(veces,0)->text().length()==0) break;
      veces++;
    }
  if (veces==ui.tableWidget->rowCount()) return veces-1;
 return veces;
}


void factura::posceldacambiada ( int row , int col, int previousRow, int previousColumn )
{
   int ultima=ultimafila();
   if (row<=ultima) return;
    if (ui.tableWidget->item(row,0)==0) ui.tableWidget->setCurrentCell(ultima,0);
      else
          if (ui.tableWidget->item(row,0)->text().length()==0)
                ui.tableWidget->setCurrentCell(ultima,0);
}


void factura::borralinea( void )
{
 int fila=ui.tableWidget->currentRow();
 ui.tableWidget->removeRow(fila);
 actualizatotales();
}

void factura::muevearriba()
{
  if (ui.tableWidget->currentRow()==0) return;

  ui.tableWidget->disconnect(SIGNAL(cellChanged(int,int)));
  ui.tableWidget->disconnect(SIGNAL(  currentCellChanged ( int , int, int, int )));
  // guardamos contenidos de la fila superior
  QString cols[ui.tableWidget->columnCount()];
  int pos=ui.tableWidget->currentRow();
  if (ui.tableWidget->item(pos,0)==0) return;

  if (ui.tableWidget->item(pos,0)->text().isEmpty()) return;

  for (int veces=0;veces<ui.tableWidget->columnCount();veces++)
  {
    cols[veces]=ui.tableWidget->item(pos-1,veces)->text();
  }


  // asignamos valores a la línea previa
  for (int veces=0;veces<ui.tableWidget->columnCount();veces++)
     ui.tableWidget->item(pos-1,veces)->setText(
                ui.tableWidget->item(pos,veces)->text());

  // asignamos valores a la línea siguiente
  for (int veces=0;veces<ui.tableWidget->columnCount();veces++)
     ui.tableWidget->item(pos,veces)->setText(cols[veces]);

  connect(ui.tableWidget,SIGNAL( cellChanged ( int , int )),this,
     SLOT(contenidocambiado(int,int )));
  connect(ui.tableWidget,SIGNAL(  currentCellChanged ( int , int, int, int )),this,
           SLOT(posceldacambiada ( int, int, int, int  )));

  int fila=pos-1;
  ui.tableWidget->setCurrentCell(fila,0);
}



// *************************************************************************************
void factura::mueveabajo()
{
    if (ui.tableWidget->currentRow()>=ui.tableWidget->rowCount()-1) return;
    if (ui.tableWidget->item(ui.tableWidget->currentRow()+1,0)==0) return;
        else if (ui.tableWidget->item(ui.tableWidget->currentRow()+1,0)->text().length()==0) return;
    if (ui.tableWidget->item(ui.tableWidget->currentRow(),0)==0) return;

  // guardamos contenidos de la fila inferior
  QString cols[ui.tableWidget->columnCount()];

  for (int veces=0;veces<ui.tableWidget->columnCount();veces++)
    cols[veces]=ui.tableWidget->item(ui.tableWidget->currentRow()+1,veces)->text();

  ui.tableWidget->disconnect(SIGNAL(cellChanged(int,int)));
  ui.tableWidget->disconnect(SIGNAL(  currentCellChanged ( int , int, int, int )));

  // asignamos valores a la fila inferior
  for (int veces=0;veces<ui.tableWidget->columnCount();veces++)
    ui.tableWidget->item(ui.tableWidget->currentRow()+1,veces)->setText(
                ui.tableWidget->item(ui.tableWidget->currentRow(),veces)->text());

  // asignamos valores (guardados) a la fila en curso
  for (int veces=0;veces<ui.tableWidget->columnCount();veces++)
    ui.tableWidget->item(ui.tableWidget->currentRow(),veces)->setText(cols[veces]);

  connect(ui.tableWidget,SIGNAL( cellChanged ( int , int )),this,
     SLOT(contenidocambiado(int,int )));
  connect(ui.tableWidget,SIGNAL(  currentCellChanged ( int , int, int, int )),this,
           SLOT(posceldacambiada ( int, int, int, int  )));

  int fila=ui.tableWidget->currentRow()+1;
  ui.tableWidget->setCurrentCell(fila,0);

}


void factura::carga_lineas_doc(QString serie,
                       QString numero)
{
    // averiguamos clave
    int clave=basedatos::instancia()->clave_doc_fac(serie,numero);

    QSqlQuery q;
    q = basedatos::instancia()->select_lin_doc(clave);
    int veces=ultimafila();
    // if (veces!=0) veces++;
    if (q.isActive())
       while (q.next())
        {
        for (int veces2=0;veces2<ui.tableWidget->columnCount();veces2++)
             if (ui.tableWidget->item(veces,veces2)==0)
                 {
                  QTableWidgetItem *newItem = new QTableWidgetItem("");
                  if (veces2==2 || veces2==3 || veces2==4 || veces2==5)
                      newItem->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
                  ui.tableWidget->setItem(veces,veces2,newItem);
                 }
        // clave, codigo, descripcion, cantidad, precio, clave_iva, tipo_iva, tipo_re, dto
        ui.tableWidget->item(veces,0)->setText(q.value(1).toString());
        ui.tableWidget->item(veces,1)->setText(q.value(2).toString());
        QString qcantidad=q.value(3).toString();
        if (qcantidad.contains(".000")) qcantidad.remove(".000");
        ui.tableWidget->item(veces,2)->setText(coma ? convacoma(qcantidad)
            : qcantidad);
        double cantidad=q.value(3).toDouble();
        double precio=q.value(4).toDouble();
        double totalneto=cantidad*precio*(1-q.value(8).toDouble()/100);
        ui.tableWidget->item(veces,3)->setText(formateanumero(q.value(4).toDouble(),coma,decimales));
        ui.tableWidget->item(veces,4)->setText(formateanumero(q.value(8).toDouble(),coma,decimales));
        ui.tableWidget->item(veces,5)->setText(formateanumero(totalneto,coma,decimales));
        ui.tableWidget->item(veces,6)->setText(q.value(5).toString());
        // tipo iva y re lugares 6 y 7
        ui.tableWidget->item(veces,7)->setText(formateanumero(q.value(6).toDouble(),
                                                              coma,decimales));
       //  QMessageBox::warning( this, tr("facturas"),formateanumero(q.value(6).toDouble(),
       //                                                           coma,decimales));

        ui.tableWidget->item(veces,8)->setText(formateanumero(q.value(7).toDouble(),
                                                              coma,decimales));
        // actualizar listas con tipos de iva y re
        fijatipoiva(q.value(5).toString(),q.value(6).toDouble()/100);
        fijatipore(q.value(5).toString(),q.value(7).toDouble()/100);

        veces++;

        }
    actualizatotales();
}


void factura::addnotas(QString notas)
{
    ui.textEdit->append("\n");
    ui.textEdit->append(notas);
}

void factura::activa_consulta()
{
  setWindowTitle(tr("DOCUMENTO: MODO CONSULTA"));
  modo_consulta=true;
  ui.doccomboBox->setEnabled(false);
  ui.serielineEdit->setReadOnly(true);
  ui.fechadateEdit->setReadOnly(true);
  ui.fechaopdateEdit->setReadOnly(true);
  ui.cta_cli_lineEdit->setReadOnly(true);
  ui.ctapushButton->setEnabled(false);
  ui.ret_checkBox->setEnabled(false);
  ui.recheckBox->setEnabled(false);
  ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui.BotonBorraLinea->setEnabled(false);
  ui.abajopushButton->setEnabled(false);
  ui.arribapushButton->setEnabled(false);
  ui.textEdit->setReadOnly(true);
  ui.pie1lineEdit->setReadOnly(true);
  ui.pie2lineEdit->setReadOnly(true);
}


void factura::pasa_fact_predef(QString codigo, QString descrip,
                               QString serie,
                               bool contabilizable,
                               bool con_ret,
                               bool re,
                               QString tipo_ret,
                               QString tipo_doc,
                               QString notas,
                               QString pie1,
                               QString pie2
                                            )
{
    predefinida=true;
    codigopredef=codigo;
    ui.documentolabel->setText(descrip);
    ui.fechalabel->hide();
    ui.fechadateEdit->hide();
    ui.fechaoplabel->hide();
    ui.fechaopdateEdit->hide();
    ui.ctaclientelabel->hide();
    ui.cta_cli_lineEdit->hide();
    ui.ctapushButton->hide();
    ui.ctapushButton->hide();
    ui.edctapushButton->hide();
    ui.clientelabel->hide();
    ui.pdfpushButton->hide();

    ui.facturalineEdit->setText(codigo);
    ui.facturalineEdit->setCursorPosition(0);
    ui.serielineEdit->setText(serie);
    contabilizable=contabilizable;
    ui.ret_checkBox->setChecked(con_ret);
    ui.recheckBox->setChecked(re);
    // ui.tipo_ret_lineEdit->setText(coma ? convacoma(tipo_ret) : tipo_ret);
    for (int veces=0; veces<ui.doccomboBox->count(); veces++)
        {
         if (ui.doccomboBox->itemText(veces)==tipo_doc)
            {
             ui.doccomboBox->setCurrentIndex(veces);
             break;
            }
        }
    ui.textEdit->setText(notas);
    ui.pie1lineEdit->setText(pie1);
    ui.pie2lineEdit->setText(pie2);


    QSqlQuery q;
    q = basedatos::instancia()->select_lin_doc_predef(codigopredef);
    int veces=ultimafila();
    // if (veces!=0) veces++;
    if (q.isActive())
       while (q.next())
        {
        for (int veces2=0;veces2<ui.tableWidget->columnCount();veces2++)
             if (ui.tableWidget->item(veces,veces2)==0)
                 {
                  QTableWidgetItem *newItem = new QTableWidgetItem("");
                  if (veces2==2 || veces2==3 || veces2==4 || veces2==5)
                      newItem->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
                  ui.tableWidget->setItem(veces,veces2,newItem);
                 }
        // codigo_doc, codigo, descripcion, cantidad, precio, clave_iva, tipo_iva, tipo_re, dto
        ui.tableWidget->item(veces,0)->setText(q.value(1).toString());
        ui.tableWidget->item(veces,1)->setText(q.value(2).toString());
        QString qcantidad=q.value(3).toString();
        if (qcantidad.contains(".000")) qcantidad.remove(".000");
        ui.tableWidget->item(veces,2)->setText(coma ? convacoma(qcantidad)
            : qcantidad);
        double cantidad=q.value(3).toDouble();
        double precio=q.value(4).toDouble();
        double totalneto=cantidad*precio*(1-q.value(8).toDouble()/100);
        ui.tableWidget->item(veces,3)->setText(formateanumero(q.value(4).toDouble(),coma,decimales));
        ui.tableWidget->item(veces,4)->setText(formateanumero(q.value(8).toDouble(),coma,decimales));
        ui.tableWidget->item(veces,5)->setText(formateanumero(totalneto,coma,decimales));
        ui.tableWidget->item(veces,6)->setText(q.value(5).toString());
        // tipo iva y re lugares 6 y 7
        ui.tableWidget->item(veces,7)->setText(formateanumero(q.value(6).toDouble(),
                                                              coma,decimales));
       //  QMessageBox::warning( this, tr("facturas"),formateanumero(q.value(6).toDouble(),
       //                                                           coma,decimales));

        ui.tableWidget->item(veces,8)->setText(formateanumero(q.value(7).toDouble(),
                                                              coma,decimales));
        // actualizar listas con tipos de iva y re
        fijatipoiva(q.value(5).toString(),q.value(6).toDouble()/100);
        fijatipore(q.value(5).toString(),q.value(7).toDouble()/100);

        veces++;

        }
    actualizatotales();



}

void factura::guarda_predef()
{
    // Comenzamos transacción: tablas facturas_predef, lin_fact_predef


   // comprobamos si existe cabecera de factura predefinida ***
    if (basedatos::instancia()->existe_fact_predef(codigopredef))
       {
        // actualizamos
        basedatos::instancia()->modificacabecerafactura_predef(
                                            codigopredef,
                                            contabilizable,
                                            ui.ret_checkBox->isChecked(),
                                            ui.recheckBox->isChecked(),
                                            convapunto(ui.tipo_ret_lineEdit->text()),
                                            ui.doccomboBox->currentText(),
                                            ui.textEdit->toPlainText(),
                                            ui.pie1lineEdit->text(),
                                            ui.pie2lineEdit->text()
                                            );
        // borramos registros de detalle
        basedatos::instancia()->borralineas_fac_predef(codigopredef);
       }
    // añadimos registros de detalle

    int ultimo=ultimafila();
    for (int veces=0; veces<ultimo; veces++)
       {
        if (ui.tableWidget->item(veces,0)==NULL) continue;
        QString tipo,re;
        cadvalorestipoiva(ui.tableWidget->item(veces,6)->text(), &tipo, &re);
        basedatos::instancia()->insert_lin_doc_predef(codigopredef,
                                       ui.tableWidget->item(veces,0)->text(),   // codigo
                                       ui.tableWidget->item(veces,1)->text(),   // descrip
                                       convapunto(ui.tableWidget->item(veces,2)->text()),   // cantidad
                                       convapunto(ui.tableWidget->item(veces,3)->text()),   // precio
                                       ui.tableWidget->item(veces,6)->text(), // clave_iva
                                       tipo, // tipo_iva
                                       re, // recargo equivalencia
                                       convapunto(ui.tableWidget->item(veces,4)->text())); // dto.
       }
    // terminamos

}
