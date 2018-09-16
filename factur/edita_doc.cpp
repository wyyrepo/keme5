/* ----------------------------------------------------------------------------------
    KEME-Contabilidad; aplicación para llevar contabilidades

    Copyright (C)   José Manuel Díez Botella

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

#include "edita_doc.h"
#include "basedatos.h"
#include "ajuste_imagen_logo.h"
#include <QFileDialog>
#include <QMessageBox>


edita_doc::edita_doc() : QDialog() {
    ui.setupUi(this);

    ui.seriecomboBox->addItems(basedatos::instancia()->listacodseries());

    // cargamos datos por defecto del emisor --------------------------------
    ui.nombre_emisorlineEdit->setText(basedatos::instancia()->selectEmpresaconfiguracion());
    ui.domicilio_emisorlineEdit->setText(basedatos::instancia()->domicilio());
    ui.poblacion_emisorlineEdit->setText(basedatos::instancia()->ciudad());
    ui.cp_emisorlineEdit->setText(basedatos::instancia()->cpostal());
    ui.provincia_emisorlineEdit->setText(basedatos::instancia()->provincia());
    ui.cif_emisorlineEdit->setText(basedatos::instancia()->cif());
    ui.id_registrallineEdit->setText(basedatos::instancia()->idregistral());

    // ----------------------------------------------------------------------

    connect(ui.aceptarpushButton, SIGNAL(clicked()), SLOT(terminar()));
    connect(ui.fotopushButton,SIGNAL( clicked()),this,SLOT(importafoto()));

    connect(ui.borrafotopushButton,SIGNAL( clicked()),this,SLOT(borrafoto()));

}

void edita_doc::pasacodigo(QString codigo)
{
    QString descrip, serie, pie1, pie2, moneda, codigo_moneda, imagen;
    bool contabilizable, rectificativo;
    int tipo_operacion;
    QString documento, cantidad, referencia, descripref, precio, totallin, bi;
    QString tipoiva, tipore, cuota, cuotare, totalfac, notas, venci, notastex;
    QString cif_empresa, cif_cliente, numero, fecha, cliente, descuento;
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

    basedatos::instancia()->carga_tipo_doc(codigo,
                                   &descrip,
                                   &serie,
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
                                   &numero, &fecha, &cliente, &descuento,
                                   &totallineas,
                                   &lineas_doc,
                                   &nombre_emisor,
                                   &domicilio_emisor,
                                   &cp_emisor,
                                   &poblacion_emisor,
                                   &provincia_emisor,
                                   &pais_emisor,
                                   &cif_emisor,
                                   &id_registral,
                                   &imagen);

    ui.codigolineEdit->setText(codigo);
    ui.descriplineEdit->setText(descrip);
    ui.pie1lineEdit->setText(pie1);
    ui.pie2lineEdit->setText(pie2);
    if (contabilizable) ui.contabilizablecheckBox->setChecked(true);
    if (rectificativo) ui.rectificativocheckBox->setChecked(true);
    for (int veces=0; veces<ui.seriecomboBox->count(); veces++)
        if (ui.seriecomboBox->itemText(veces)==serie)
           {
             ui.seriecomboBox->setCurrentIndex(veces);
             break;
           }

    for (int veces=0; veces<ui.simcomboBox->count(); veces++)
        if (ui.simcomboBox->itemText(veces)==moneda)
           {
             ui.simcomboBox->setCurrentIndex(veces);
             break;
           }

    for (int veces=0; veces<ui.codcomboBox->count(); veces++)
        if (ui.codcomboBox->itemText(veces)==codigo_moneda)
           {
             ui.codcomboBox->setCurrentIndex(veces);
             break;
           }

/*    &tipo_operacion,
    &documento, &cantidad, &referencia, &descripref,
    &precio, &totallin, &bi,
    &tipoiva, &tipore, &cuota, &cuotare, &totalfac,
    &notas, &venci, */


    for (int veces=0; veces<ui.operacioncomboBox->count(); veces++)
        if (ui.operacioncomboBox->itemText(veces).left(2).toInt()==tipo_operacion)
           {
             ui.operacioncomboBox->setCurrentIndex(veces);
             break;
           }
    if (!documento.isEmpty())
     {
      ui.documentolineEdit->setText(documento);
      ui.cantidadlineEdit->setText(cantidad);
      ui.referencialineEdit->setText(referencia);
      ui.descripreflineEdit->setText(descripref);
      ui.preciolineEdit->setText(precio);
      ui.totallineEdit->setText(totallin);
      ui.bilineEdit->setText(bi);
      ui.tipoivalineEdit->setText(tipoiva);
      ui.tiporelineEdit->setText(tipore);
      ui.cuotalineEdit->setText(cuota);
      ui.cuotarelineEdit->setText(cuotare);
      ui.totalfaclineEdit->setText(totalfac);
      ui.notaslineEdit->setText(notas);
      ui.vencilineEdit->setText(venci);
      ui.cif_empresalineEdit->setText(cif_empresa);
      ui.cif_clientelineEdit->setText(cif_cliente);
      ui.numerolineEdit->setText(numero);
      ui.fechalineEdit->setText(fecha);
      ui.clientelineEdit->setText(cliente);
      ui.descuentolineEdit->setText(descuento);
      ui.totallineaslineEdit->setText(totallineas);
      ui.lineasspinBox->setValue(lineas_doc.toInt());
      ui.nombre_emisorlineEdit->setText(nombre_emisor);
      ui.domicilio_emisorlineEdit->setText(domicilio_emisor);
      ui.cp_emisorlineEdit->setText(cp_emisor);
      ui.poblacion_emisorlineEdit->setText(poblacion_emisor);
      ui.provincia_emisorlineEdit->setText(provincia_emisor);
      ui.pais_emisorlineEdit->setText(pais_emisor);
      ui.cif_emisorlineEdit->setText(cif_emisor);
      ui.id_registrallineEdit->setText(id_registral);
     }
    ui.notastextEdit->setText(notastex);
    QPixmap foto;
    if (imagen.length()>0)
          {
           QByteArray byteshexa;
           byteshexa.append ( imagen );
           QByteArray bytes;
           bytes=bytes.fromHex ( byteshexa );
           foto.loadFromData ( bytes, "PNG");
          }
    ui.logo_label->setPixmap(foto);

    ui.pie1lineEdit->setText(pie1);
    ui.pie2lineEdit->setText(pie2);

}


void edita_doc::terminar()
{
    QString tipooperacion=ui.operacioncomboBox->currentText().left(2);
    QString lineas_doc; lineas_doc.setNum(ui.lineasspinBox->value());
    basedatos::instancia()->guarda_tipo_doc(ui.codigolineEdit->text(),
                                   ui.descriplineEdit->text(),
                                   ui.seriecomboBox->currentText(),
                                   ui.pie1lineEdit->text(),
                                   ui.pie2lineEdit->text(),
                                   ui.simcomboBox->currentText(),
                                   ui.codcomboBox->currentText(),
                                   ui.contabilizablecheckBox->isChecked(),
                                   ui.rectificativocheckBox->isChecked(),
                                   tipooperacion,
                                   ui.documentolineEdit->text(),
                                   ui.cantidadlineEdit->text(),
                                   ui.referencialineEdit->text(),
                                   ui.descripreflineEdit->text(),
                                   ui.preciolineEdit->text(),
                                   ui.totallineEdit->text(),
                                   ui.bilineEdit->text(),
                                   ui.tipoivalineEdit->text(),
                                   ui.tiporelineEdit->text(),
                                   ui.cuotalineEdit->text(),
                                   ui.cuotarelineEdit->text(),
                                   ui.totalfaclineEdit->text(),
                                   ui.notaslineEdit->text(),
                                   ui.vencilineEdit->text(),
                                   ui.notastextEdit->toPlainText(),
                                   ui.cif_empresalineEdit->text(),
                                   ui.cif_clientelineEdit->text(),
                                   ui.numerolineEdit->text(),
                                   ui.fechalineEdit->text(),
                                   ui.clientelineEdit->text(),
                                   ui.descuentolineEdit->text(),
                                   ui.totallineaslineEdit->text(),
                                   lineas_doc,
                                   ui.nombre_emisorlineEdit->text(),
                                   ui.domicilio_emisorlineEdit->text(),
                                   ui.cp_emisorlineEdit->text(),
                                   ui.poblacion_emisorlineEdit->text(),
                                   ui.provincia_emisorlineEdit->text(),
                                   ui.pais_emisorlineEdit->text(),
                                   ui.cif_emisorlineEdit->text(),
                                   ui.id_registrallineEdit->text()
                                   );


    basedatos::instancia()->grabaimagenlogo(ui.codigolineEdit->text(), fototexto());
    accept();
}



void edita_doc::importafoto()
{
  QFileDialog dialogofich(this);
  dialogofich.setFileMode(QFileDialog::ExistingFile);
  dialogofich.setLabelText ( QFileDialog::LookIn, tr("Directorio:") );
  dialogofich.setLabelText ( QFileDialog::FileName, tr("Archivo:") );
  dialogofich.setLabelText ( QFileDialog::FileType, tr("Tipo de archivo:") );
  dialogofich.setLabelText ( QFileDialog::Accept, tr("Aceptar") );
  dialogofich.setLabelText ( QFileDialog::Reject, tr("Cancelar") );

  QStringList filtros;
  filtros << tr("Archivos de gráficos (*.png *.jpg *.bmp)");
  dialogofich.setNameFilters(filtros);
  dialogofich.setDirectory(getenv("HOME"));
  dialogofich.setWindowTitle(tr("SELECCIÓN DE FICHERO GRÁFICO"));
  // dialogofich.exec();
  //QString fileName = dialogofich.getOpenFileName(this, tr("Seleccionar archivo para importar asientos"),
  //                                              dirtrabajo,
  //                                              tr("Ficheros de texto (*.txt)"));
  QStringList fileNames;
  if (dialogofich.exec())
     {
      fileNames = dialogofich.selectedFiles();
      if (fileNames.at(0).length()>0)
          {
           QString cadfich=fileNames.at(0);
           QPixmap imagen(cadfich);
           // ----------------------------------------------------------------------
           ajuste_imagen_logo *a = new ajuste_imagen_logo(imagen);
           int resultado=a->exec();
           if (resultado==QDialog::Accepted)
               ui.logo_label->setPixmap(a->seleccion());
             else
              {
              // QMessageBox::information( this, tr("SUBCUENTAS"),tr("ACEPTADO"));
              // ----------------------------------------------------------------------
               int altolabel=ui.logo_label->height();
               int ancholabel=ui.logo_label->width();
               int altopix=imagen.height();
               int anchopix=imagen.width();
               QPixmap definitiva;
               if (altopix/altolabel>anchopix/ancholabel)
                 definitiva=imagen.scaledToHeight(altolabel);
               else
                 definitiva=imagen.scaledToWidth(ancholabel);
               ui.logo_label->setScaledContents(false);
               ui.logo_label->setPixmap(definitiva);
              }
           delete(a);
          }
     }

}

QString edita_doc::fototexto()
{
 if (ui.logo_label->pixmap()==0) return "";
 QPixmap foto=*ui.logo_label->pixmap();
 QByteArray bytes;
 QBuffer buffer(&bytes);
 buffer.open(QIODevice::WriteOnly);
 foto.save(&buffer, "PNG"); // writes pixmap into foto in PNG format
 QByteArray cadhexa=bytes.toHex();
 QString cadena(cadhexa);

 return cadena;

}

void edita_doc::borrafoto()
{
  if (QMessageBox::question(
            this,
            tr("¿ Borrar imagen ?"),
            tr("¿ Desea borrar la imagen actual ?"),
            tr("&Sí"), tr("&No"),
            QString::null, 0, 1 ) ==0 )
               {
                QPixmap foto;
                ui.logo_label->setPixmap(foto);
               }
}

