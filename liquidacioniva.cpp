/* ----------------------------------------------------------------------------------
    KEME-Contabilidad; aplicación para llevar contabilidades

    Copyright (C)  José Manuel Díez Botella

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

#include "liquidacioniva.h"
#include "funciones.h"
#include "basedatos.h"
#include "tabla_asientos.h"
#include <QProgressDialog>
#include <QMessageBox>

liquidacioniva::liquidacioniva(QString qusuario) : QDialog() {
    ui.setupUi(this);


  ui.inicialdateEdit->setDate(QDate::currentDate());
  ui.finaldateEdit->setDate(QDate::currentDate());
usuario=qusuario;
QStringList columnas;
 columnas << tr("Cuenta") << tr("Base imponible") << tr("Tipo");
 columnas << tr("Cuota") << tr("Total");
 columnas << tr("B.Inversión") << tr("Cuota efectiva") << tr("Cuota C.Caja");
  ui.soportadotable->setHorizontalHeaderLabels(columnas);

  ui.imp_soportadotable->setHorizontalHeaderLabels(columnas);

QStringList columnas3;
 columnas3 << tr("Cuenta") << tr("Base imponible") << tr("Tipo");
 columnas3 << tr("Cuota sop.") << tr("Cuota rep.");
 columnas3 << tr("B.Inversión");
 ui.aibtable->setHorizontalHeaderLabels(columnas3);

  QStringList columnas2;
 columnas2 << tr("Cuenta") << tr("Base imponible") << tr("Tipo");
 columnas2 << tr("Cuota") << tr("T.R.E.") << tr("R.E.") << tr("Cuota C.Caja");
  ui.repercutidotable->setHorizontalHeaderLabels(columnas2);


  QStringList columnas4;
   columnas4 << tr("Cuenta") << tr("Base imponible") << tr("Tipo");
   columnas4 << tr("Cuota sop.") << tr("Cuota rep.");
   ui.autofacturastable->setHorizontalHeaderLabels(columnas4);

   QStringList columnas5;
    columnas5 << tr("Cuenta") << tr("Base imponible") << tr("Tipo");
    columnas5 << tr("Cuota sop.") << tr("Cuota rep.");
    ui.autofacturas_exttable->setHorizontalHeaderLabels(columnas5);

    ui.autofacturas_interiorestable->setHorizontalHeaderLabels(columnas5);

    QStringList columnas6;
     columnas6 << tr("Cuenta") << tr("Base imponible") << tr("Tipo");
     columnas6 << tr("Cuota") << tr("TOTAL");
     ui.agrariotableWidget->setHorizontalHeaderLabels(columnas6);


if (conigic())
   {
    ui.textLabel4->setText(tr("IGIC SOPORTADO"));
    ui.cuotasoptextLabel->setText(tr("Cuota total IGIC soportado"));
    ui.textLabel5->setText(tr("IGIC REPERCUTIDO"));
    ui.textLabel6->setText(tr("TOTAL IGIC"));
    setWindowTitle(tr("Liquidación de IGIC"));
   }

  comadecimal=haycomadecimal(); decimales=haydecimales();

  // tabla_iva_sop *ts_caja_corr, *ts_caja_inv
  ts_caja_corr=new tabla_iva_sop(usuario,comadecimal,decimales);
  ts_caja_inv=new tabla_iva_sop(usuario,comadecimal,decimales);
  ts_caja_rep=new tabla_iva_rep(usuario,comadecimal,decimales);

  connect(ui.calcularpushButton,SIGNAL( clicked()),this,SLOT(calcular()));
  // connect(ui.imprimepushButton,SIGNAL( clicked()),this,SLOT(()));
  connect(ui.inicialdateEdit,SIGNAL(dateChanged(QDate)),this,SLOT(fechacambiada()));
  connect(ui.finaldateEdit,SIGNAL(dateChanged(QDate)),this,SLOT(fechacambiada()));

  connect(ui.imprimepushButton,SIGNAL( clicked()),this,SLOT(imprimir()));
  connect(ui.latexpushButton,SIGNAL(clicked()),SLOT(latex()));

  connect(ui.copiapushButton,SIGNAL(clicked()),SLOT(copia()));

  connect(ui.caja_corrientespushButton,SIGNAL(clicked()),SLOT(ver_iva_caja_soportado_corrientes()));
  connect(ui.caja_inversionpushButton,SIGNAL(clicked()),SLOT(ver_iva_caja_soportado_inversion()));
  connect(ui.iva_caja_reppushButton,SIGNAL(clicked()),SLOT(ver_iva_caja_repercutido()));

  connect(ui.asientopushButton,SIGNAL(clicked()),SLOT(gen_asiento()));

  ui.concepto_liq_label->setText(" - - ");

#ifdef NOEDITTEX
  ui.latexpushButton->hide();
#endif

}

liquidacioniva::~liquidacioniva()
{
    delete(ts_caja_corr);
    delete(ts_caja_inv);
    delete(ts_caja_rep);
}

void liquidacioniva::calcular()
{
  borrar();
  
  ts_caja_corr->pasa_info_liquidacion(ui.inicialdateEdit->date(),
                                ui.finaldateEdit->date(),false);
  // QMessageBox::information( this, tr("INFO"),"primera consulta");

  ts_caja_inv->pasa_info_liquidacion(ui.inicialdateEdit->date(),
                                ui.finaldateEdit->date(),true);
  // QMessageBox::information( this, tr("INFO"),"2 consulta");

  // ----------------------------------------------------

  ts_caja_rep->pasa_info_liquidacion(ui.inicialdateEdit->date(),
                                ui.finaldateEdit->date());
  // QMessageBox::information( this, tr("INFO"),"3 consulta");

  ui.caja_corrientespushButton->setEnabled(true);
  ui.caja_inversionpushButton->setEnabled(true);
  ui.iva_caja_reppushButton->setEnabled(true);

  // QMessageBox::information( this, tr("INFO"),"inicio");


  QProgressDialog progress(tr("Calculando liquidación ..."), 0, 0, 31, this);
  progress.setMinimumDuration ( 100 );
  progress.setValue(1);
  QApplication::processEvents();


  QSqlQuery consulta = basedatos::instancia()->libroivafechas_sop_corrientes( ui.inicialdateEdit->date(),
                                                                            ui.finaldateEdit->date() );

  int fila=0;
  double suma=0;
  while (consulta.next())
      {
         ui.soportadotable->insertRow(fila);
         ui.soportadotable->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));
         QTableWidgetItem *newItem1 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
         newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.soportadotable->setItem(fila,1,newItem1);

         QString tipoiva=consulta.value(1).toString();
         if (comadecimal) tipoiva=convacoma(tipoiva);
         QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
         newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.soportadotable->setItem(fila,2,newItem2);

         
         QTableWidgetItem *newItem3 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(3).toDouble(),comadecimal,decimales));
         newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.soportadotable->setItem(fila,3,newItem3);

         QTableWidgetItem *newItem4 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(4).toDouble(),comadecimal,decimales));
         newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.soportadotable->setItem(fila,4,newItem4);

         QTableWidgetItem *newItem5 = new QTableWidgetItem(QString());
         newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
         ui.soportadotable->setItem(fila,5,newItem5);

         QTableWidgetItem *newItem6 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(6).toDouble(),comadecimal,decimales));
         newItem6->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.soportadotable->setItem(fila,6,newItem6);

         ui.soportadotable->setItem(fila,7,new QTableWidgetItem(QString()));

         if (consulta.value(3).toDouble() - consulta.value(5).toDouble()>0.001)
             ui.cuotasoptextLabel->setText(tr("Cuota iva soportado corregida por prorrata/no afectación:"));
         suma+=redond(consulta.value(6).toDouble(),2);
         QApplication::processEvents();
         fila++;
      }

  // ---------------------------------------------------------
  // QMessageBox::information( this, tr("INFO"),"sop corrientes");
  // ---------------------------------------------------------
  progress.setValue(2);
  QApplication::processEvents();

  consulta = basedatos::instancia()->libroivafechas_sop_inversion( ui.inicialdateEdit->date(),
                                                                            ui.finaldateEdit->date() );
  while (consulta.next())
      {
         ui.soportadotable->insertRow(fila);
         ui.soportadotable->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));
         QTableWidgetItem *newItem1 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
         newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.soportadotable->setItem(fila,1,newItem1);

         QString tipoiva=consulta.value(1).toString();
         if (comadecimal) tipoiva=convacoma(tipoiva);
         QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
         newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.soportadotable->setItem(fila,2,newItem2);


         QTableWidgetItem *newItem3 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(3).toDouble(),comadecimal,decimales));
         newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.soportadotable->setItem(fila,3,newItem3);

         QTableWidgetItem *newItem4 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(4).toDouble(),comadecimal,decimales));
         newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.soportadotable->setItem(fila,4,newItem4);

         QTableWidgetItem *newItem5 = new QTableWidgetItem("*");
         newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
         ui.soportadotable->setItem(fila,5,newItem5);

         QTableWidgetItem *newItem6 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(6).toDouble(),comadecimal,decimales));
         newItem6->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.soportadotable->setItem(fila,6,newItem6);

         ui.soportadotable->setItem(fila,7,new QTableWidgetItem(QString()));

         if (consulta.value(3).toDouble() - consulta.value(6).toDouble()>0.001)
             ui.cuotasoptextLabel->setText(tr("Cuota iva soportado corregida por prorrata/no afectación:"));
         suma+=redond(consulta.value(5).toDouble(),2);
         fila++;
         QApplication::processEvents();
      }

  // ---------------------------------------------------------
  // QMessageBox::information( this, tr("INFO"),"sop inversion");
  // ---------------------------------------------------------
  progress.setValue(3);
  QApplication::processEvents();

  // vamos con el IVA soportado de caja bienes corrientes
  QStringList selcuenta_iva, seltipo;
  QList<double> selbi,selcuota,seltotal,selcuota_efectiva,selcuota_liquidada;

  ts_caja_corr->calcula_listas_sel();

  selcuenta_iva=ts_caja_corr->qselcuenta_iva();
  seltipo=ts_caja_corr->qseltipo();
  selbi=ts_caja_corr->qselbi();
  selcuota=ts_caja_corr->qselcuota();
  seltotal=ts_caja_corr->qseltotal();
  selcuota_efectiva=ts_caja_corr->qselcuota_efectiva();
  selcuota_liquidada=ts_caja_corr->qselcuota_liquidada();

  for (int veces=0; veces<selcuenta_iva.count(); veces++)
    {
      ui.soportadotable->insertRow(fila);
      ui.soportadotable->setItem(fila,0,new QTableWidgetItem(selcuenta_iva.at(veces)));
      QTableWidgetItem *newItem1 = new QTableWidgetItem(
                   formatea_redondeado(selbi.at(veces),comadecimal,decimales));
      newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.soportadotable->setItem(fila,1,newItem1);

      QString tipoiva=seltipo.at(veces);
      if (comadecimal) tipoiva=convacoma(tipoiva);
      QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
      newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.soportadotable->setItem(fila,2,newItem2);


      QTableWidgetItem *newItem3 = new QTableWidgetItem(
                   formatea_redondeado(selcuota.at(veces),comadecimal,decimales));
      newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.soportadotable->setItem(fila,3,newItem3);

      QTableWidgetItem *newItem4 = new QTableWidgetItem(
                   formatea_redondeado(seltotal.at(veces),comadecimal,decimales));
      newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.soportadotable->setItem(fila,4,newItem4);

      QTableWidgetItem *newItem5 = new QTableWidgetItem(""); // no es inversión
      newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
      ui.soportadotable->setItem(fila,5,newItem5);

      QTableWidgetItem *newItem6 = new QTableWidgetItem(
                   formatea_redondeado(selcuota_efectiva.at(veces),comadecimal,decimales));
      newItem6->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.soportadotable->setItem(fila,6,newItem6);

      QTableWidgetItem *newItem7 = new QTableWidgetItem(
                   formatea_redondeado(selcuota_liquidada.at(veces),comadecimal,decimales));
      newItem7->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.soportadotable->setItem(fila,7,newItem7);

      if (selcuota.at(veces) - selcuota_efectiva.at(veces)>0.001)
          ui.cuotasoptextLabel->setText(tr("Cuota iva soportado corregida por prorrata/no afectación:"));
      suma+=redond(selcuota_liquidada.at(veces),2);
      fila++;
      QApplication::processEvents();
    }

  progress.setValue(4);
  QApplication::processEvents();

  // vamos con el IVA soportado de caja bienes inversion
  selcuenta_iva.clear();
  seltipo.clear();
  selbi.clear();
  selcuota.clear();
  seltotal.clear();
  selcuota_efectiva.clear();
  selcuota_liquidada.clear();

  ts_caja_inv->calcula_listas_sel();

  selcuenta_iva=ts_caja_inv->qselcuenta_iva();
  seltipo=ts_caja_inv->qseltipo();
  selbi=ts_caja_inv->qselbi();
  selcuota=ts_caja_inv->qselcuota();
  seltotal=ts_caja_inv->qseltotal();
  selcuota_efectiva=ts_caja_inv->qselcuota_efectiva();
  selcuota_liquidada=ts_caja_inv->qselcuota_liquidada();

  for (int veces=0; veces<selcuenta_iva.count(); veces++)
    {
      ui.soportadotable->insertRow(fila);
      ui.soportadotable->setItem(fila,0,new QTableWidgetItem(selcuenta_iva.at(veces)));
      QTableWidgetItem *newItem1 = new QTableWidgetItem(
                   formatea_redondeado(selbi.at(veces),comadecimal,decimales));
      newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.soportadotable->setItem(fila,1,newItem1);

      QString tipoiva=seltipo.at(veces);
      if (comadecimal) tipoiva=convacoma(tipoiva);
      QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
      newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.soportadotable->setItem(fila,2,newItem2);


      QTableWidgetItem *newItem3 = new QTableWidgetItem(
                   formatea_redondeado(selcuota.at(veces),comadecimal,decimales));
      newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.soportadotable->setItem(fila,3,newItem3);

      QTableWidgetItem *newItem4 = new QTableWidgetItem(
                   formatea_redondeado(seltotal.at(veces),comadecimal,decimales));
      newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.soportadotable->setItem(fila,4,newItem4);

      QTableWidgetItem *newItem5 = new QTableWidgetItem("*"); // es inversión
      newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
      ui.soportadotable->setItem(fila,5,newItem5);

      QTableWidgetItem *newItem6 = new QTableWidgetItem(
                   formatea_redondeado(selcuota_efectiva.at(veces),comadecimal,decimales));
      newItem6->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.soportadotable->setItem(fila,6,newItem6);

      QTableWidgetItem *newItem7 = new QTableWidgetItem(
                   formatea_redondeado(selcuota_liquidada.at(veces),comadecimal,decimales));
      newItem7->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.soportadotable->setItem(fila,7,newItem7);

      if (selcuota.at(veces) - selcuota_efectiva.at(veces)>0.001)
          ui.cuotasoptextLabel->setText(tr("Cuota iva soportado corregida por prorrata/no afectación:"));
      suma+=redond(selcuota_liquidada.at(veces),2);
      fila++;
      QApplication::processEvents();
    }



  progress.setValue(5);
  QApplication::processEvents();

  // calculamos ahora IVA soportado al tipo 0 (exento) e insertamos fila

    consulta = basedatos::instancia()->baserecibidasexentas_corrientes(
          ui.inicialdateEdit->date() , ui.finaldateEdit->date() );

 if (consulta.next() && !(consulta.value(0).toDouble()<0.0001 && consulta.value(0).toDouble()>-0.0001))
  {
    ui.soportadotable->insertRow(fila);
    ui.soportadotable->setItem(fila,0,new QTableWidgetItem(tr("B.I. al tipo 0")));
    QTableWidgetItem *newItem1 = new QTableWidgetItem(
                   formatea_redondeado(consulta.value(0).toDouble(),comadecimal,decimales));
    newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
    ui.soportadotable->setItem(fila,1,newItem1);

    QTableWidgetItem *newItem2 = new QTableWidgetItem("");
    ui.soportadotable->setItem(fila,2,newItem2);

    QTableWidgetItem *newItem3 = new QTableWidgetItem("");
    ui.soportadotable->setItem(fila,3,newItem3);

    QTableWidgetItem *newItem4 = new QTableWidgetItem("");
    ui.soportadotable->setItem(fila,4,newItem4);

    QTableWidgetItem *newItem5 = new QTableWidgetItem(QString());
    newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
    ui.soportadotable->setItem(fila,5,newItem5);

    ui.soportadotable->setItem(fila,6,new QTableWidgetItem(QString()));
    ui.soportadotable->setItem(fila,7,new QTableWidgetItem(QString()));

    fila++;
    QApplication::processEvents();
  }

 progress.setValue(6);
 QApplication::processEvents();

  // calculamos ahora IVA soportado al tipo 0 (exento) de INVERSIONES e insertamos fila

    consulta = basedatos::instancia()->baserecibidasexentas_inversion(
          ui.inicialdateEdit->date() , ui.finaldateEdit->date() );

 if (consulta.next() && !(consulta.value(0).toDouble()<0.0001 && consulta.value(0).toDouble()>-0.0001))
  {
    ui.soportadotable->insertRow(fila);
    ui.soportadotable->setItem(fila,0,new QTableWidgetItem(tr("B.I. al tipo 0")));
    QTableWidgetItem *newItem1 = new QTableWidgetItem(
                   formatea_redondeado(consulta.value(0).toDouble(),comadecimal,decimales));
    newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
    ui.soportadotable->setItem(fila,1,newItem1);

    QTableWidgetItem *newItem2 = new QTableWidgetItem("");
    ui.soportadotable->setItem(fila,2,newItem2);

    QTableWidgetItem *newItem3 = new QTableWidgetItem("");
    ui.soportadotable->setItem(fila,3,newItem3);

    QTableWidgetItem *newItem4 = new QTableWidgetItem("");
    ui.soportadotable->setItem(fila,4,newItem4);

    QTableWidgetItem *newItem5 = new QTableWidgetItem("*");
    newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
    ui.soportadotable->setItem(fila,5,newItem5);
    ui.soportadotable->setItem(fila,6,new QTableWidgetItem(QString()));
    ui.soportadotable->setItem(fila,7,new QTableWidgetItem(QString()));

    fila++;
  }

  double sumaliquidacion=0;
  if (suma>0.001 || suma<-0.001)
     {
      ui.cuotalineEdit->setText(formatea_redondeado(suma,comadecimal,decimales));
      sumaliquidacion+=suma;
     }

  // --------------------------------------------------------------------------------------
  // ---------------soportando en importación----------------------------------------------
  progress.setValue(7);
  QApplication::processEvents();

  consulta = basedatos::instancia()->libroivafechas_sop_corrientes_importacion
                 ( ui.inicialdateEdit->date(),
                  ui.finaldateEdit->date() );
  fila=0;
  suma=0;
  while (consulta.next())
      {
         ui.imp_soportadotable->insertRow(fila);
         ui.imp_soportadotable->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));
         QTableWidgetItem *newItem1 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
         newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.imp_soportadotable->setItem(fila,1,newItem1);

         QString tipoiva=consulta.value(1).toString();
         if (comadecimal) tipoiva=convacoma(tipoiva);
         QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
         newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.imp_soportadotable->setItem(fila,2,newItem2);


         QTableWidgetItem *newItem3 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(3).toDouble(),comadecimal,decimales));
         newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.imp_soportadotable->setItem(fila,3,newItem3);

         QTableWidgetItem *newItem4 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(4).toDouble(),comadecimal,decimales));
         newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.imp_soportadotable->setItem(fila,4,newItem4);

         QTableWidgetItem *newItem5 = new QTableWidgetItem(QString());
         newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
         ui.imp_soportadotable->setItem(fila,5,newItem5);

         QTableWidgetItem *newItem6 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(6).toDouble(),comadecimal,decimales));
         newItem6->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.imp_soportadotable->setItem(fila,6,newItem6);


         if (consulta.value(3).toDouble() - consulta.value(5).toDouble()>0.001)
             ui.cuotasop_imptextLabel->setText(tr("Cuota iva soportado corregida por prorrata/no afectación:"));
         suma+=redond(consulta.value(6).toDouble(),2);
         fila++;
         QApplication::processEvents();
      }

  progress.setValue(8);
  QApplication::processEvents();


  consulta = basedatos::instancia()->libroivafechas_sop_inversion_importacion(
          ui.inicialdateEdit->date(),
          ui.finaldateEdit->date() );
  while (consulta.next())
      {
         ui.imp_soportadotable->insertRow(fila);
         ui.imp_soportadotable->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));
         QTableWidgetItem *newItem1 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
         newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.imp_soportadotable->setItem(fila,1,newItem1);

         QString tipoiva=consulta.value(1).toString();
         if (comadecimal) tipoiva=convacoma(tipoiva);
         QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
         newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.imp_soportadotable->setItem(fila,2,newItem2);


         QTableWidgetItem *newItem3 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(3).toDouble(),comadecimal,decimales));
         newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.imp_soportadotable->setItem(fila,3,newItem3);

         QTableWidgetItem *newItem4 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(4).toDouble(),comadecimal,decimales));
         newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.imp_soportadotable->setItem(fila,4,newItem4);

         QTableWidgetItem *newItem5 = new QTableWidgetItem("*");
         newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
         ui.imp_soportadotable->setItem(fila,5,newItem5);

         QTableWidgetItem *newItem6 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(6).toDouble(),comadecimal,decimales));
         newItem6->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.imp_soportadotable->setItem(fila,6,newItem6);

         if (consulta.value(3).toDouble() - consulta.value(5).toDouble()>0.001)
             ui.cuotasop_imptextLabel->setText(tr("Cuota iva soportado corregida por prorrata/no afectación:"));
         suma+=redond(consulta.value(6).toDouble(),2);
         fila++;
         QApplication::processEvents();
      }

  progress.setValue(9);
  QApplication::processEvents();

  // calculamos ahora IVA soportado al tipo 0 (exento) e insertamos fila

    consulta = basedatos::instancia()->baserecibidasexentas_corrientes_importacion(
          ui.inicialdateEdit->date() , ui.finaldateEdit->date() );

 if (consulta.next() && !(consulta.value(0).toDouble()<0.0001 && consulta.value(0).toDouble()>-0.0001))
  {
    ui.imp_soportadotable->insertRow(fila);
    ui.imp_soportadotable->setItem(fila,0,new QTableWidgetItem(tr("B.I. al tipo 0")));
    QTableWidgetItem *newItem1 = new QTableWidgetItem(
                   formatea_redondeado(consulta.value(0).toDouble(),comadecimal,decimales));
    newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
    ui.imp_soportadotable->setItem(fila,1,newItem1);

    QTableWidgetItem *newItem2 = new QTableWidgetItem("");
    ui.imp_soportadotable->setItem(fila,2,newItem2);

    QTableWidgetItem *newItem3 = new QTableWidgetItem("");
    ui.imp_soportadotable->setItem(fila,3,newItem3);

    QTableWidgetItem *newItem4 = new QTableWidgetItem("");
    ui.imp_soportadotable->setItem(fila,4,newItem4);

    QTableWidgetItem *newItem5 = new QTableWidgetItem(QString());
    newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
    ui.imp_soportadotable->setItem(fila,5,newItem5);
    fila++;
    QApplication::processEvents();
  }

 progress.setValue(10);
 QApplication::processEvents();

  // calculamos ahora IVA soportado al tipo 0 (exento) de INVERSIONES e insertamos fila

    consulta = basedatos::instancia()->baserecibidasexentas_inversion_importacion(
          ui.inicialdateEdit->date() , ui.finaldateEdit->date() );

 if (consulta.next() && !(consulta.value(0).toDouble()<0.0001 && consulta.value(0).toDouble()>-0.0001))
  {
    ui.imp_soportadotable->insertRow(fila);
    ui.imp_soportadotable->setItem(fila,0,new QTableWidgetItem(tr("B.I. al tipo 0")));
    QTableWidgetItem *newItem1 = new QTableWidgetItem(
                   formatea_redondeado(consulta.value(0).toDouble(),comadecimal,decimales));
    newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
    ui.imp_soportadotable->setItem(fila,1,newItem1);

    QTableWidgetItem *newItem2 = new QTableWidgetItem("");
    ui.imp_soportadotable->setItem(fila,2,newItem2);

    QTableWidgetItem *newItem3 = new QTableWidgetItem("");
    ui.imp_soportadotable->setItem(fila,3,newItem3);

    QTableWidgetItem *newItem4 = new QTableWidgetItem("");
    ui.imp_soportadotable->setItem(fila,4,newItem4);

    QTableWidgetItem *newItem5 = new QTableWidgetItem("*");
    newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
    ui.imp_soportadotable->setItem(fila,5,newItem5);
    fila++;
    QApplication::processEvents();
  }

  if (suma>0.001 || suma<-0.001)
     {
      ui.cuota_implineEdit->setText(formatea_redondeado(suma,comadecimal,decimales));
      sumaliquidacion+=suma;
     }

  progress.setValue(11);
  QApplication::processEvents();


  // AQUÍ TERMINA SOPORTADO EN IMPORTACIONES
  // --------------------------------------------------------------------------------------



  // ---------------------------------------------------------------------------------------
  //   REPERCUTIDO
  // ---------------------------------------------------------------------------------------
  progress.setValue(12);
  QApplication::processEvents();

  fila=0;

  consulta = basedatos::instancia()->select7DiarioLibroivafechasgroupcuentatipo_ivatipo_re(
          ui.inicialdateEdit->date() , ui.finaldateEdit->date() );

  while (consulta.next())
      {
         ui.repercutidotable->insertRow(fila);
         ui.repercutidotable->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));
         QTableWidgetItem *newItem1 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
         newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.repercutidotable->setItem(fila,1,newItem1);

         QString tipoiva=consulta.value(1).toString();
         if (comadecimal) tipoiva=convacoma(tipoiva);
         QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
         newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.repercutidotable->setItem(fila,2,newItem2);

         double num;
         num=consulta.value(3).toDouble();
         QTableWidgetItem *newItem3 = new QTableWidgetItem(
                      formatea_redondeado(num,comadecimal,decimales));
         newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.repercutidotable->setItem(fila,3,newItem3);

         QString tipore=consulta.value(4).toString();
         if (comadecimal) tipore=convacoma(tipore);
         QTableWidgetItem *newItem4 = new QTableWidgetItem(tipore);
         newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.repercutidotable->setItem(fila,4,newItem4); // tipo re

         QTableWidgetItem *newItem5 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(5).toDouble(),comadecimal,decimales));
         newItem5->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.repercutidotable->setItem(fila,5,newItem5); // cuota re
         ui.repercutidotable->setItem(fila,6,new QTableWidgetItem("")); // cuota caja vacía
         fila++;
         QApplication::processEvents();
      }
  // ---------------------------------------------------------------------------------
  // Repercutido, criterio de caja
  progress.setValue(13);
  QApplication::processEvents();
  selcuenta_iva.clear();
  seltipo.clear();
  selbi.clear();
  selcuota.clear();
  seltotal.clear();
  selcuota_liquidada.clear();

  ts_caja_rep->calcula_listas_sel();

  selcuenta_iva=ts_caja_rep->qselcuenta_iva();
  seltipo=ts_caja_rep->qseltipo();
  selbi=ts_caja_rep->qselbi();
  selcuota=ts_caja_rep->qselcuota();
  seltotal=ts_caja_rep->qseltotal();
  selcuota_liquidada=ts_caja_rep->qselcuota_liquidada();

  for (int veces=0; veces<selcuenta_iva.count(); veces++)
    {
      ui.repercutidotable->insertRow(fila);
      ui.repercutidotable->setItem(fila,0,new QTableWidgetItem(selcuenta_iva.at(veces)));
      QTableWidgetItem *newItem1 = new QTableWidgetItem(
                   formatea_redondeado(selbi.at(veces),comadecimal,decimales));
      newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.repercutidotable->setItem(fila,1,newItem1);

      QString tipoiva=seltipo.at(veces);
      if (comadecimal) tipoiva=convacoma(tipoiva);
      QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
      newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.repercutidotable->setItem(fila,2,newItem2);


      QTableWidgetItem *newItem3 = new QTableWidgetItem(
                   formatea_redondeado(selcuota.at(veces),comadecimal,decimales));
      newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.repercutidotable->setItem(fila,3,newItem3);

      ui.repercutidotable->setItem(fila,4,new QTableWidgetItem(""));
      ui.repercutidotable->setItem(fila,5,new QTableWidgetItem(""));

      QTableWidgetItem *newItem7 = new QTableWidgetItem(
                   formatea_redondeado(selcuota_liquidada.at(veces),comadecimal,decimales));
      newItem7->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
      ui.repercutidotable->setItem(fila,6,newItem7);

      fila++;
      QApplication::processEvents();
    }

  progress.setValue(14);
  QApplication::processEvents();

  // -----------------------------------------------------------------------------
  suma=0;
  double suma2=0;
  for (int veces=0;veces<ui.repercutidotable->rowCount();veces++)
    {
      QString cuota=convapunto(ui.repercutidotable->item(veces,3)->text());
      QString cuota_caja=convapunto(ui.repercutidotable->item(veces,6)->text());
      suma+= cuota_caja.isEmpty() ? cuota.toDouble() : cuota_caja.toDouble();
      suma2+=convapunto(ui.repercutidotable->item(veces,5)->text()).toDouble();
    }
  if (suma>0.001 || suma<-0.001)
    {
      ui.ivalineEdit->setText(formatea_redondeado(suma,comadecimal,decimales));
      sumaliquidacion-=suma;
    }
  if (suma2>0.001 || suma2<-0.001)
    {
      ui.relineEdit->setText(formatea_redondeado(suma2,comadecimal,decimales));
      sumaliquidacion-=suma2;
    }
  // for (int veces=1;veces<5;veces++) soportadotable->adjustColumn(veces);
  // for (int veces=1;veces<6;veces++) repercutidotable->adjustColumn(veces);

  // Base imponible exenta de emitidas no eib
  /*----------------------------------------------------------------------------
    consulta = basedatos::instancia()->baseemitidasexentasnoeib (
          ui.inicialdateEdit->date() , ui.finaldateEdit->date() );
   if (consulta.next() && !(consulta.value(0).toDouble()<0.0001 && consulta.value(0).toDouble()>-0.0001))
   {
    ui.repercutidotable->insertRow(fila);
    ui.repercutidotable->setItem(fila,0,new QTableWidgetItem(tr("B.I. al tipo 0")));
    QTableWidgetItem *newItem1 = new QTableWidgetItem(
                   formatea_redondeado(consulta.value(0).toDouble(),comadecimal,decimales));
    newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
    ui.repercutidotable->setItem(fila,1,newItem1);

    QTableWidgetItem *newItem2 = new QTableWidgetItem("");
    ui.repercutidotable->setItem(fila,2,newItem2);

    QTableWidgetItem *newItem3 = new QTableWidgetItem("");
    ui.repercutidotable->setItem(fila,3,newItem3);

    QTableWidgetItem *newItem4 = new QTableWidgetItem("");
    ui.repercutidotable->setItem(fila,4,newItem4);

    QTableWidgetItem *newItem5 = new QTableWidgetItem(QString());
    newItem5->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
    ui.repercutidotable->setItem(fila,5,newItem5);
   }
  ------------------------------------------------------------------------------------------*/
  progress.setValue(15);
  QApplication::processEvents();

  consulta = basedatos::instancia()->libroiva_aib_corrientes( ui.inicialdateEdit->date() ,
                                                              ui.finaldateEdit->date() );
  fila=0;

  while (consulta.next())
      {
         ui.aibtable->insertRow(fila);
         ui.aibtable->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));

         QTableWidgetItem *newItem1 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
         newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,1,newItem1);

         QString tipoiva=consulta.value(1).toString();
         if (comadecimal) tipoiva=convacoma(tipoiva);
         QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
         newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,2,newItem2);

         QTableWidgetItem *newItem3 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(3).toDouble(),comadecimal,decimales));
         sumaliquidacion+=consulta.value(3).toDouble();
         newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,3,newItem3);
         QTableWidgetItem *newItem4 = new QTableWidgetItem("");
         newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,4,newItem4);
         QTableWidgetItem *newItem5 = new QTableWidgetItem("");
         newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,5,newItem5);
         fila++;
      }

  // ----------------------------------------------------------------------------------------------
  progress.setValue(16);
  QApplication::processEvents();

  consulta = basedatos::instancia()->libroiva_aib_inversion( ui.inicialdateEdit->date() ,
                                                              ui.finaldateEdit->date() );

  while (consulta.next())
      {
         ui.aibtable->insertRow(fila);
         ui.aibtable->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));

         QTableWidgetItem *newItem1 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
         newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,1,newItem1);

         QString tipoiva=consulta.value(1).toString();
         if (comadecimal) tipoiva=convacoma(tipoiva);
         QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
         newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,2,newItem2);

         QTableWidgetItem *newItem3 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(3).toDouble(),comadecimal,decimales));
         sumaliquidacion+=consulta.value(3).toDouble();
         newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,3,newItem3);
         QTableWidgetItem *newItem4 = new QTableWidgetItem("");
         newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,4,newItem4);
         QTableWidgetItem *newItem5 = new QTableWidgetItem("*");
         newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,5,newItem5);
         fila++;
      }

  progress.setValue(17);
  QApplication::processEvents();

  // ----------------------------------------------------------------------------------------------
  // Esto es para la parte de repercutido de las aib
  consulta = basedatos::instancia()->select4DiarioLibroivafechasgroupcuentatipo_ivasinprorrata(
          ui.inicialdateEdit->date() , ui.finaldateEdit->date() );

  while (consulta.next())
      {
         ui.aibtable->insertRow(fila);
         ui.aibtable->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));

         QTableWidgetItem *newItem1 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
         newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,1,newItem1);

         QString tipoiva=consulta.value(1).toString();
         if (comadecimal) tipoiva=convacoma(tipoiva);
         QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
         newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,2,newItem2);

         QTableWidgetItem *newItem3 = new QTableWidgetItem("");
         newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,3,newItem3);
         QTableWidgetItem *newItem4 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(3).toDouble(),comadecimal,decimales));
         sumaliquidacion-=consulta.value(3).toDouble();
         newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,4,newItem4);
         QTableWidgetItem *newItem5 = new QTableWidgetItem("");
         newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
         ui.aibtable->setItem(fila,5,newItem5);
         fila++;
      }

  // AIB exentas a incluir aquí
  progress.setValue(18);
  QApplication::processEvents();

    consulta = basedatos::instancia()->baseemitidasexentasaib_corrientes (
          ui.inicialdateEdit->date() , ui.finaldateEdit->date() );
   if (consulta.next() && !(consulta.value(0).toDouble()<0.0001 && consulta.value(0).toDouble()>-0.0001))
   {
    ui.aibtable->insertRow(fila);
    ui.aibtable->setItem(fila,0,new QTableWidgetItem(tr("B.I. al tipo 0")));
    QTableWidgetItem *newItem1 = new QTableWidgetItem(
                   formatea_redondeado(consulta.value(0).toDouble(),comadecimal,decimales));
    newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
    ui.aibtable->setItem(fila,1,newItem1);

    QTableWidgetItem *newItem2 = new QTableWidgetItem("");
    ui.aibtable->setItem(fila,2,newItem2);

    QTableWidgetItem *newItem3 = new QTableWidgetItem("");
    ui.aibtable->setItem(fila,3,newItem3);

    QTableWidgetItem *newItem4 = new QTableWidgetItem("");
    ui.aibtable->setItem(fila,4,newItem4);

    QTableWidgetItem *newItem5 = new QTableWidgetItem("");
    ui.aibtable->setItem(fila,5,newItem5);
   }



  // AIB exentas inversión a incluir aquí
   progress.setValue(19);
   QApplication::processEvents();

    consulta = basedatos::instancia()->baseemitidasexentasaib_inversion (
          ui.inicialdateEdit->date() , ui.finaldateEdit->date() );
   if (consulta.next() && !(consulta.value(0).toDouble()<0.0001 && consulta.value(0).toDouble()>-0.0001))
   {
    ui.aibtable->insertRow(fila);
    ui.aibtable->setItem(fila,0,new QTableWidgetItem(tr("B.I. al tipo 0")));
    QTableWidgetItem *newItem1 = new QTableWidgetItem(
                   formatea_redondeado(consulta.value(0).toDouble(),comadecimal,decimales));
    newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
    ui.aibtable->setItem(fila,1,newItem1);

    QTableWidgetItem *newItem2 = new QTableWidgetItem("");
    ui.aibtable->setItem(fila,2,newItem2);

    QTableWidgetItem *newItem3 = new QTableWidgetItem("");
    ui.aibtable->setItem(fila,3,newItem3);

    QTableWidgetItem *newItem4 = new QTableWidgetItem("");
    ui.aibtable->setItem(fila,4,newItem4);

    QTableWidgetItem *newItem5 = new QTableWidgetItem("*");
    ui.aibtable->setItem(fila,5,newItem5);
   }


  // -------------------------------------------------------------------------------------
  // suma base eib
  consulta = basedatos::instancia()->selectSumbase_ivalibroivadiariofechas( ui.inicialdateEdit->date() , ui.finaldateEdit->date() );
  fila=0;
  if (consulta.next())  ui.eiblineEdit->setText(formatea_redondeado(consulta.value(0).toDouble(),
                                                 comadecimal,decimales));
  progress.setValue(20);
  QApplication::processEvents();

  // llenamos cuadro autofacturas ue
  consulta = basedatos::instancia()->libroiva_autofacturas_ue( ui.inicialdateEdit->date() ,
                                                              ui.finaldateEdit->date() );
  while (consulta.next())
      {
         ui.autofacturastable->insertRow(fila);
         ui.autofacturastable->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));

         QTableWidgetItem *newItem1 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
         newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturastable->setItem(fila,1,newItem1);

         QString tipoiva=consulta.value(1).toString();
         if (comadecimal) tipoiva=convacoma(tipoiva);
         QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
         newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturastable->setItem(fila,2,newItem2);

         QTableWidgetItem *newItem3 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(3).toDouble(),comadecimal,decimales));
         sumaliquidacion+=consulta.value(3).toDouble();
         newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturastable->setItem(fila,3,newItem3);
         QTableWidgetItem *newItem4 = new QTableWidgetItem("");
         newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturastable->setItem(fila,4,newItem4);
         fila++;
      }

  // llenamos cuadro autofacturas ue parte repercutido
  consulta = basedatos::instancia()->libroiva_autofacturas_ue_emitidas( ui.inicialdateEdit->date(),
                                                              ui.finaldateEdit->date() );
  progress.setValue(21);
  QApplication::processEvents();

  while (consulta.next())
      {
         ui.autofacturastable->insertRow(fila);
         ui.autofacturastable->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));

         QTableWidgetItem *newItem1 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
         newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturastable->setItem(fila,1,newItem1);

         QString tipoiva=consulta.value(1).toString();
         if (comadecimal) tipoiva=convacoma(tipoiva);
         QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
         newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturastable->setItem(fila,2,newItem2);

         QTableWidgetItem *newItem3 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(3).toDouble(),comadecimal,decimales));
         sumaliquidacion-=consulta.value(3).toDouble();
         newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturastable->setItem(fila,4,newItem3);
         QTableWidgetItem *newItem4 = new QTableWidgetItem("");
         newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturastable->setItem(fila,3,newItem4);
         fila++;
      }


  // calculamos ahora IVA soportado por autofacturas al tipo 0 (exento) e insertamos fila
    consulta = basedatos::instancia()->baseautofacturasrecibidasexentas(
          ui.inicialdateEdit->date() , ui.finaldateEdit->date() );

    progress.setValue(22);
    QApplication::processEvents();

 if (consulta.next() && !(consulta.value(0).toDouble()<0.0001 && consulta.value(0).toDouble()>-0.0001))
  {
    ui.autofacturastable->insertRow(fila);
    ui.autofacturastable->setItem(fila,0,new QTableWidgetItem(tr("Autofacturas UE al tipo 0")));
    QTableWidgetItem *newItem1 = new QTableWidgetItem(
                   formatea_redondeado(consulta.value(0).toDouble(),comadecimal,decimales));
    newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
    ui.autofacturastable->setItem(fila,1,newItem1);

    QTableWidgetItem *newItem2 = new QTableWidgetItem("");
    ui.autofacturastable->setItem(fila,2,newItem2);

    QTableWidgetItem *newItem3 = new QTableWidgetItem("");
    ui.autofacturastable->setItem(fila,3,newItem3);

    QTableWidgetItem *newItem4 = new QTableWidgetItem("");
    ui.autofacturastable->setItem(fila,4,newItem4);
    // QMessageBox::information( this, tr("INFO"),consulta.value(0).toString());

    QTableWidgetItem *newItem5 = new QTableWidgetItem(QString());
    newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
    ui.autofacturastable->setItem(fila,5,newItem5);
    fila++;
  }

  // Autofacturas no ue

  fila=0;
  progress.setValue(23);
  QApplication::processEvents();

  // llenamos cuadro autofacturas fuera ue
  consulta = basedatos::instancia()->libroiva_autofacturas_no_ue( ui.inicialdateEdit->date() ,
                                                              ui.finaldateEdit->date() );
  while (consulta.next())
      {
         ui.autofacturas_exttable->insertRow(fila);
         ui.autofacturas_exttable->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));

         QTableWidgetItem *newItem1 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
         newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturas_exttable->setItem(fila,1,newItem1);

         QString tipoiva=consulta.value(1).toString();
         if (comadecimal) tipoiva=convacoma(tipoiva);
         QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
         newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturas_exttable->setItem(fila,2,newItem2);

         QTableWidgetItem *newItem3 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(3).toDouble(),comadecimal,decimales));
         sumaliquidacion+=consulta.value(3).toDouble();
         newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturas_exttable->setItem(fila,3,newItem3);
         QTableWidgetItem *newItem4 = new QTableWidgetItem("");
         newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturas_exttable->setItem(fila,4,newItem4);
         fila++;
      }

  // llenamos cuadro autofacturas fuera ue parte repercutido
  consulta = basedatos::instancia()->libroiva_autofacturas_no_ue_emitidas( ui.inicialdateEdit->date(),
                                                              ui.finaldateEdit->date() );
  progress.setValue(24);
  QApplication::processEvents();
  while (consulta.next())
      {
         ui.autofacturas_exttable->insertRow(fila);
         ui.autofacturas_exttable->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));

         QTableWidgetItem *newItem1 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
         newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturas_exttable->setItem(fila,1,newItem1);

         QString tipoiva=consulta.value(1).toString();
         if (comadecimal) tipoiva=convacoma(tipoiva);
         QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
         newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturas_exttable->setItem(fila,2,newItem2);

         QTableWidgetItem *newItem3 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(3).toDouble(),comadecimal,decimales));
         sumaliquidacion-=consulta.value(3).toDouble();
         newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturas_exttable->setItem(fila,4,newItem3);
         QTableWidgetItem *newItem4 = new QTableWidgetItem("");
         newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.autofacturas_exttable->setItem(fila,3,newItem4);
         fila++;
      }

  progress.setValue(25);
  QApplication::processEvents();


  // calculamos ahora IVA soportado por autofacturas no ue al tipo 0 (exento) e insertamos fila
    consulta = basedatos::instancia()->baseautofacturasnouerecibidasexentas(
          ui.inicialdateEdit->date() , ui.finaldateEdit->date() );

 if (consulta.next() && !(consulta.value(0).toDouble()<0.0001 && consulta.value(0).toDouble()>-0.0001))
  {
    ui.autofacturas_exttable->insertRow(fila);
    ui.autofacturas_exttable->setItem(fila,0,new QTableWidgetItem(tr("Autofacturas no UE al tipo 0")));
    QTableWidgetItem *newItem1 = new QTableWidgetItem(
                   formatea_redondeado(consulta.value(0).toDouble(),comadecimal,decimales));
    newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
    ui.autofacturas_exttable->setItem(fila,1,newItem1);

    QTableWidgetItem *newItem2 = new QTableWidgetItem("");
    ui.autofacturas_exttable->setItem(fila,2,newItem2);

    QTableWidgetItem *newItem3 = new QTableWidgetItem("");
    ui.autofacturas_exttable->setItem(fila,3,newItem3);

    QTableWidgetItem *newItem4 = new QTableWidgetItem("");
    ui.autofacturas_exttable->setItem(fila,4,newItem4);

    QTableWidgetItem *newItem5 = new QTableWidgetItem(QString());
    newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
    ui.autofacturas_exttable->setItem(fila,5,newItem5);
  }

  // --------------------------------------------------------------------------------------
  // -------- ISP OPERACIONES INTERIORES

 // Autofacturas no ue

 fila=0;
 progress.setValue(26);
 QApplication::processEvents();

 // llenamos cuadro ISP interiores
 consulta = basedatos::instancia()->libroiva_isp_interiores_recibidas( ui.inicialdateEdit->date() ,
                                                             ui.finaldateEdit->date() );
 while (consulta.next())
     {
        ui.autofacturas_interiorestable->insertRow(fila);
        ui.autofacturas_interiorestable->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));

        QTableWidgetItem *newItem1 = new QTableWidgetItem(
                     formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
        newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
        ui.autofacturas_interiorestable->setItem(fila,1,newItem1);

        QString tipoiva=consulta.value(1).toString();
        if (comadecimal) tipoiva=convacoma(tipoiva);
        QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
        newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
        ui.autofacturas_interiorestable->setItem(fila,2,newItem2);

        QTableWidgetItem *newItem3 = new QTableWidgetItem(
                     formatea_redondeado(consulta.value(3).toDouble(),comadecimal,decimales));
        sumaliquidacion+=consulta.value(3).toDouble();
        newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
        ui.autofacturas_interiorestable->setItem(fila,3,newItem3);
        QTableWidgetItem *newItem4 = new QTableWidgetItem("");
        newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
        ui.autofacturas_interiorestable->setItem(fila,4,newItem4);
        fila++;
     }

 // llenamos cuadro autofacturas fuera ue parte repercutido
 progress.setValue(27);
 QApplication::processEvents();
 consulta = basedatos::instancia()->libroiva_isp_interiores_emitidas(
                  ui.inicialdateEdit->date(),
                  ui.finaldateEdit->date() );
 while (consulta.next())
     {
        ui.autofacturas_interiorestable->insertRow(fila);
        ui.autofacturas_interiorestable->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));

        QTableWidgetItem *newItem1 = new QTableWidgetItem(
                     formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
        newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
        ui.autofacturas_interiorestable->setItem(fila,1,newItem1);

        QString tipoiva=consulta.value(1).toString();
        if (comadecimal) tipoiva=convacoma(tipoiva);
        QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
        newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
        ui.autofacturas_interiorestable->setItem(fila,2,newItem2);

        QTableWidgetItem *newItem3 = new QTableWidgetItem(
                     formatea_redondeado(consulta.value(3).toDouble(),comadecimal,decimales));
        sumaliquidacion-=consulta.value(3).toDouble();
        newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
        ui.autofacturas_interiorestable->setItem(fila,4,newItem3);
        QTableWidgetItem *newItem4 = new QTableWidgetItem("");
        newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
        ui.autofacturas_interiorestable->setItem(fila,3,newItem4);
        fila++;
     }

 progress.setValue(28);
 QApplication::processEvents();


 // calculamos ahora IVA soportado por isp interiores al tipo 0 (exento) e insertamos fila
   consulta = basedatos::instancia()->base_isp_interiores_recibidasexentas(
         ui.inicialdateEdit->date() , ui.finaldateEdit->date() );

if (consulta.next() && !(consulta.value(0).toDouble()<0.0001 && consulta.value(0).toDouble()>-0.0001))
 {
   ui.autofacturas_interiorestable->insertRow(fila);
   ui.autofacturas_interiorestable->setItem(fila,0,new QTableWidgetItem(tr("Autofacturas no UE al tipo 0")));
   QTableWidgetItem *newItem1 = new QTableWidgetItem(
                  formatea_redondeado(consulta.value(0).toDouble(),comadecimal,decimales));
   newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
   ui.autofacturas_interiorestable->setItem(fila,1,newItem1);

   QTableWidgetItem *newItem2 = new QTableWidgetItem("");
   ui.autofacturas_interiorestable->setItem(fila,2,newItem2);

   QTableWidgetItem *newItem3 = new QTableWidgetItem("");
   ui.autofacturas_interiorestable->setItem(fila,3,newItem3);

   QTableWidgetItem *newItem4 = new QTableWidgetItem("");
   ui.autofacturas_interiorestable->setItem(fila,4,newItem4);

   QTableWidgetItem *newItem5 = new QTableWidgetItem(QString());
   newItem5->setTextAlignment (Qt::AlignCenter | Qt::AlignVCenter);
   ui.autofacturas_interiorestable->setItem(fila,5,newItem5);
 }

progress.setValue(29);
QApplication::processEvents();

  // -------------------------------------------------------------------------------------
  // suma base eib
  consulta = basedatos::instancia()->selectSumbase_serv_ue_libroivadiariofechas( ui.inicialdateEdit->date() , ui.finaldateEdit->date() );
  fila=0;
  if (consulta.next())  ui.pr_servlineEdit->setText(formatea_redondeado(consulta.value(0).toDouble(),
                                                 comadecimal,decimales));

  // --------------------------------------------------------------------------------------
  // EXPORTACIONES

  consulta = basedatos::instancia()->selectSumbase_expor_fechas ( ui.inicialdateEdit->date(),
                                                                  ui.finaldateEdit->date() );
  fila=0;
  if (consulta.next())  ui.exportlineEdit->setText(formatea_redondeado(consulta.value(0).toDouble(),
                                                 comadecimal,decimales));

  progress.setValue(30);
  QApplication::processEvents();

  // EMITIDAS EXENTAS CON DCHO A DEDUCCIÓN Y SIN DERECHO A DEDUCCIÓN

  consulta = basedatos::instancia()->baseemitidasexentas_noeib_noexport  (
        ui.inicialdateEdit->date() , ui.finaldateEdit->date() );

  double exentas_todas=0;

  if (consulta.next()) exentas_todas=consulta.value(0).toDouble();

  consulta = basedatos::instancia()->baseemitidasexentas_noeib_noexport_nodeduc   (
        ui.inicialdateEdit->date() , ui.finaldateEdit->date() );

  double exentas_no_deduc=0;

  if (consulta.next()) exentas_no_deduc=consulta.value(0).toDouble();

  ui.exentas_deduclineEdit->setText(formatea_redondeado(exentas_todas-exentas_no_deduc,
                                                   comadecimal,decimales));

  ui.exentas_no_deduclineEdit->setText(formatea_redondeado(exentas_no_deduc,
                                                      comadecimal,decimales));
  progress.setValue(31);
  QApplication::processEvents();

  // -------------------------------------------------------------------------------------
  consulta = basedatos::instancia()->libroivafechas_sop_agrario (
                                                       ui.inicialdateEdit->date(),
                                                       ui.finaldateEdit->date() );

  // RÉGIMEN AGRARIO
  fila=0;
  while (consulta.next())
      {
         ui.agrariotableWidget->insertRow(fila);
         ui.agrariotableWidget->setItem(fila,0,new QTableWidgetItem(consulta.value(0).toString()));
         // base
         QTableWidgetItem *newItem1 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(2).toDouble(),comadecimal,decimales));
         newItem1->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.agrariotableWidget->setItem(fila,1,newItem1);

         QString tipoiva=consulta.value(1).toString();
         if (comadecimal) tipoiva=convacoma(tipoiva);
         QTableWidgetItem *newItem2 = new QTableWidgetItem(tipoiva);
         newItem2->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.agrariotableWidget->setItem(fila,2,newItem2);

         // cuota
         QTableWidgetItem *newItem3 = new QTableWidgetItem(
                      formatea_redondeado(consulta.value(3).toDouble(),comadecimal,decimales));
         newItem3->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.agrariotableWidget->setItem(fila,3,newItem3);
         double imp_total=consulta.value(2).toDouble() + consulta.value(3).toDouble();
         sumaliquidacion+=consulta.value(3).toDouble();
         QTableWidgetItem *newItem4 = new QTableWidgetItem(formatea_redondeado(imp_total,
                                                           comadecimal,decimales));
         newItem4->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         ui.agrariotableWidget->setItem(fila,4,newItem4);
         fila++;
      }
  if (sumaliquidacion>0)
    {
     ui.concepto_liq_label->setText(tr("A compensar/devolver"));
    }
    else
         {
          ui.concepto_liq_label->setText(tr("A ingresar en Tesoro"));
         }
 ui.resultadolineEdit->setText(formatea_redondeado(sumaliquidacion,
                                              comadecimal,decimales));
}


void liquidacioniva::borrar()
{
  ui.soportadotable->setRowCount(0);
  ui.repercutidotable->setRowCount(0);
  ui.aibtable->setRowCount(0);
  ui.autofacturastable->setRowCount(0);
  ui.autofacturas_exttable->setRowCount(0);
  ui.imp_soportadotable->setRowCount(0);
  ui.autofacturas_interiorestable->setRowCount(0);
  ui.cuota_implineEdit->clear();
  ui.cuotalineEdit->clear();
  ui.ivalineEdit->clear();
  ui.relineEdit->clear();
  ui.eiblineEdit->clear();
  ui.exportlineEdit->clear();
  ui.pr_servlineEdit->clear();
  ui.resultadolineEdit->clear();
  ui.concepto_liq_label->setText(" - - ");
}


void liquidacioniva::fechacambiada()
{
  borrar();
}


void liquidacioniva::generalatex()
{
   QString qfichero=dirtrabajo();
   qfichero.append(QDir::separator());
   qfichero=qfichero+tr("liquidacion_IVA.tex");
   QFile fichero(adapta(qfichero));
   if (! fichero.open( QIODevice::WriteOnly ) ) return;

    QTextStream stream( &fichero );
    stream.setCodec("UTF-8");

    stream << cabeceralatex();
    stream << margen_extra_latex();

    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {" << filtracad(nombreempresa()) << "}}" << "\n";
    stream << "\\end{center}" << "\n";
    stream << "\n";
    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {" << tr("LIQUIDACIÓN DE IVA") <<  "}}" << "\n";
    stream << "\\end{center}" << "\n";
    stream << "\\begin{center}" << "\n";
    stream << "{\\textbf {" << tr("Período de ") << ui.inicialdateEdit->date().toString("dd.MM.yyyy") <<
    tr(" a ") << ui.finaldateEdit->date().toString("dd.MM.yyyy") << "}}" << "\n";
    stream << "\\end{center}" << "\n";
    
    stream << "\\\n";


    stream << "\\begin{center}\n";
    stream << "{\\textbf {" << tr("RESULTADO LIQUIDACIÓN: ") <<
          ui.concepto_liq_label->text() << " "
         << formatea_redondeado_sep(convapunto(ui.resultadolineEdit->text()).toDouble(),
                              comadecimal,decimales) <<  "}}" << "\n";
    stream << "\\end{center}\n";

    stream << "\\\n";



    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {" << tr("IVA REPERCUTIDO") <<  "}}" << "\n";
    stream << "\\end{center}" << "\n";

     // --------------------------------------------------------------------------------------------------
    stream << "\\begin{center}\n";
    stream << "\\begin{tabular}{|c|r|r|r|r|r|r|r|}\n";
    stream << "\\hline\n";
    stream << "{\\scriptsize " << tr("CUENTA") << "} & {\\scriptsize " << tr("Base Imponible") << 
      "} & {\\scriptsize " << tr("Tipo") << "} & {\\scriptsize " << tr("Cuota") << "} & {\\scriptsize " << 
      tr("T.R.E.") << "} & {\\scriptsize " << tr("R.E.") << "} & {\\scriptsize "
      << tr("Total")  << "} & {\\scriptsize "  << tr("Cuota C.Caja") << "}  \\\\\n";
    stream << "\\hline\n";
    int pos=0;
    double tcuotare=0;
    double tbi=0; double tcuota=0; double ttotal=0; double tcuotacaja=0;
    while (pos<ui.repercutidotable->rowCount())
        {
          double bi,cuota,cuotare,total;
          stream << "{\\scriptsize " << ui.repercutidotable->item(pos,0)->text() << 
		  "} & {\\scriptsize " <<
                      formatea_redondeado_sep(convapunto(ui.repercutidotable->item(pos,1)->text()).toDouble(),
                      comadecimal,decimales) << 
		  "} & {\\scriptsize ";
          bi=convapunto(ui.repercutidotable->item(pos,1)->text()).toDouble();
          stream << formatea_redondeado_sep(convapunto(ui.repercutidotable->item(pos,2)->text()).toDouble(),
                   comadecimal,decimales) << "} & {\\scriptsize ";
          if (ui.repercutidotable->item(pos,6)->text().isEmpty())
             stream << formatea_redondeado_sep(convapunto(ui.repercutidotable->item(pos,3)->text()).toDouble(),
                   comadecimal,decimales);
          stream << "} & {\\scriptsize " <<
               formatea_redondeado_sep(convapunto(ui.repercutidotable->item(pos,4)->text()).toDouble(),
                   comadecimal,decimales) << "} & {\\scriptsize ";
         cuota=convapunto(ui.repercutidotable->item(pos,3)->text()).toDouble();
         stream << formatea_redondeado_sep(convapunto(ui.repercutidotable->item(pos,5)->text()).toDouble(),
                   comadecimal,decimales);
         stream << "} & {\\scriptsize ";
         cuotare=convapunto(ui.repercutidotable->item(pos,5)->text()).toDouble();
         total=bi+cuota+cuotare;
         stream << formatea_redondeado_sep(total,comadecimal,decimales) << "} & {\\scriptsize ";
         stream << formatea_redondeado_sep(convapunto(ui.repercutidotable->item(pos,6)->text()).toDouble(),
                                           comadecimal, decimales);
         tbi+=bi;
         if (ui.repercutidotable->item(pos,6)->text().isEmpty())
            tcuota+=cuota;
         tcuotare+=cuotare;
         ttotal+=total;
         tcuotacaja+=convapunto(ui.repercutidotable->item(pos,6)->text()).toDouble();
         stream << "} \\\\ \n " << "\\hline\n";
         pos++;
       }

    stream << "{\\scriptsize \\textbf {";
    stream << tr("Totales:");
    stream << "}}";
    // colocamos línea de totales
    stream << " & {\\scriptsize ";
    stream << formatea_redondeado_sep(tbi,comadecimal,decimales);
    stream << "} & & ";

    // ahora viene la cuota
    stream << "{\\scriptsize ";
    stream << formatea_redondeado_sep(tcuota,comadecimal,decimales);
    // stream << "} & {\\scriptsize ";


    stream << "} & & ";

    // ahora viene la cuota RE
    stream << "{\\scriptsize ";
    stream << formatea_redondeado_sep(tcuotare,comadecimal,decimales);
    stream << "} & {\\scriptsize ";

    // total
    stream << formatea_redondeado_sep(ttotal,comadecimal,decimales);

    stream << "} & {\\scriptsize ";

    // tcuotacaja
    stream << formatea_redondeado_sep(tcuotacaja,comadecimal,decimales);

    stream << "} \\\\ \n";
    stream << "\\hline\n";



    stream << "\\end{tabular}\n";
    stream << "\\end{center}\n";

    stream << "\\begin{center}\n";
    stream << "{\\textbf {" << tr("Total IVA repercutido:  ") << formatea_redondeado_sep(convapunto(ui.ivalineEdit->text()).toDouble(),
                   comadecimal,decimales) << 
            tr(" - Total recargo equivalencia: ") << formatea_redondeado_sep(convapunto(ui.relineEdit->text()).toDouble(),
                   comadecimal,decimales) << "}}" << "\n";
    stream << "\\end{center}\n";
    
    stream << "\\\n";
    stream << "\\\n";
    //----------------------------------------------------------------------------------------------------------

    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {" << tr("IVA SOPORTADO") <<  "}}" << "\n";
    stream << "\\end{center}" << "\n";

     // ----------------------------------------------------------------------------------------------
    stream << "\\begin{center}\n";
    stream << "\\begin{tabular}{|c|r|r|r|r|c|r|r|}\n";
    stream << "\\hline\n";
    stream << "{\\scriptsize " << tr("CUENTA") << "} & {\\scriptsize " << tr("Base Imponible") <<
      "} & {\\scriptsize " << tr("Tipo") << "} & {\\scriptsize " << tr("Cuota") << "} & {\\scriptsize " <<
      tr("Total") << "} & {\\scriptsize " << tr("Inversión")
      << "} & {\\scriptsize " << tr("Cuota efectiva")
      << "} & {\\scriptsize " << tr("Cuota C.Caja") << "}  \\\\\n";
    stream << "\\hline\n";
    pos=0;
    tbi=0; tcuota=0; ttotal=0; double tcuota_efectiva=0; double tcuota_caja=0;
    while (pos<ui.soportadotable->rowCount())
        {
          stream << "{\\scriptsize " << ui.soportadotable->item(pos,0)->text() <<
                  "} & {\\scriptsize " << formatea_redondeado_sep(convapunto(
                                 ui.soportadotable->item(pos,1)->text()).toDouble(),comadecimal,decimales) <<
                  "} & {\\scriptsize ";
          stream << formatea_redondeado_sep(convapunto(
                       ui.soportadotable->item(pos,2)->text()).toDouble(),comadecimal,decimales)
                       << "} & {\\scriptsize ";
          if (ui.soportadotable->item(pos,7)->text().isEmpty())
             stream << formatea_redondeado_sep(convapunto(ui.soportadotable->item(pos,3)->text()).toDouble(),
                                     comadecimal,decimales);
          stream << "} & {\\scriptsize ";
          // stream << convacoma(soportadotable->text(pos,4));
          stream << formatea_redondeado_sep(convapunto(ui.soportadotable->item(pos,4)->text()).toDouble(),
                                     comadecimal,decimales);
          QString estrella=(ui.soportadotable->item(pos,5)!=0) ? ui.soportadotable->item(pos,5)->text() : "";
          stream << "} & {\\scriptsize " << estrella << "} & {\\scriptsize ";
          if (ui.soportadotable->item(pos,7)->text().isEmpty())
            {
             if (ui.soportadotable->item(pos,6)!=NULL)
              stream << formatea_redondeado_sep(convapunto(ui.soportadotable->item(pos,6)->text()).toDouble(),
                                     comadecimal,decimales);
            }
         tbi+= convapunto(ui.soportadotable->item(pos,1)->text()).toDouble();
         if (ui.soportadotable->item(pos,7)->text().isEmpty())
            tcuota+=convapunto(ui.soportadotable->item(pos,3)->text()).toDouble();
         ttotal+=convapunto(ui.soportadotable->item(pos,4)->text()).toDouble();
         if (ui.soportadotable->item(pos,7)->text().isEmpty())
            tcuota_efectiva+= ui.soportadotable->item(pos,6)!=NULL ?
                           convapunto(ui.soportadotable->item(pos,6)->text()).toDouble() : 0;
         tcuota_caja+=convapunto(ui.soportadotable->item(pos,7)->text()).toDouble();
         stream << "} & {\\scriptsize ";
         stream << formatea_redondeado_sep(convapunto(ui.soportadotable->item(pos,7)->text()).toDouble(),
                                    comadecimal,decimales);
         stream << "} \\\\ \n " << "\\hline\n";
         // QMessageBox::information( this, tr("INFO"),ui.soportadotable->item(pos,0)->text());
         pos++;
       }
    stream << "{\\scriptsize \\textbf {";
    stream << tr("Totales:");
    stream << "}}";
    // colocamos línea de totales
    stream << " & {\\scriptsize ";
    stream << formatea_redondeado_sep(tbi,comadecimal,decimales);
    stream << "} & & ";

    // ahora viene la cuota
    stream << "{\\scriptsize ";
    stream << formatea_redondeado_sep(tcuota,comadecimal,decimales);
    stream << "} & {\\scriptsize ";

    // total
    stream << formatea_redondeado_sep(ttotal,comadecimal,decimales);
    stream << "} & & {\\scriptsize ";
    stream << formatea_redondeado_sep(tcuota_efectiva,comadecimal,decimales);
    stream << "} &  {\\scriptsize ";
    stream << formatea_redondeado_sep(tcuota_caja,comadecimal,decimales);
    stream << "} \\\\ \n";
    stream << "\\hline\n";


    stream << "\\end{tabular}\n";
    stream << "\\end{center}\n";
    stream << "\\begin{center}\n";
    stream << "{\\textbf {" << ui.cuotasoptextLabel->text() << " " <<
           formatea_redondeado_sep(convapunto(ui.cuotalineEdit->text()).toDouble(),comadecimal,decimales) << "}}" << "\n";
    stream << "\\end{center}\n";

    stream << "\\\n";
    stream << "\\\n";


   // ----------------------------------------------------------------------------------
   // --- IVA SOPORTADO EN IMPORTACIONES


    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {" << tr("IVA SOPORTADO IMPORTACIONES") <<  "}}" << "\n";
    stream << "\\end{center}" << "\n";

     // ----------------------------------------------------------------------------------------------
    stream << "\\begin{center}\n";
    stream << "\\begin{tabular}{|c|r|r|r|r|c|r|}\n";
    stream << "\\hline\n";
    stream << "{\\scriptsize " << tr("CUENTA") << "} & {\\scriptsize " << tr("Base Imponible") <<
      "} & {\\scriptsize " << tr("Tipo") << "} & {\\scriptsize " << tr("Cuota") << "} & {\\scriptsize " <<
      tr("Total") << "} & {\\scriptsize " << tr("Inversión")
      << "} & {\\scriptsize " << tr("Cuota efectiva") << "}  \\\\\n";
    stream << "\\hline\n";
    pos=0;
    tbi=0, tcuota=0, ttotal=0, tcuota_efectiva=0;
    while (pos<ui.imp_soportadotable->rowCount())
        {
          stream << "{\\scriptsize " << ui.imp_soportadotable->item(pos,0)->text() <<
                  "} & {\\scriptsize " << formatea_redondeado_sep(convapunto(
                                 ui.imp_soportadotable->item(pos,1)->text()).toDouble(),comadecimal,decimales) <<
                  "} & {\\scriptsize ";
          stream << formatea_redondeado_sep(convapunto(
                       ui.imp_soportadotable->item(pos,2)->text()).toDouble(),comadecimal,decimales)
                       << "} & {\\scriptsize " <<
                    formatea_redondeado_sep(convapunto(ui.imp_soportadotable->item(pos,3)->text()).toDouble(),
                                     comadecimal,decimales) << "} & {\\scriptsize ";
          // stream << convacoma(soportadotable->text(pos,4));
          stream << formatea_redondeado_sep(convapunto(ui.imp_soportadotable->item(pos,4)->text()).toDouble(),
                                     comadecimal,decimales);
          QString estrella=(ui.imp_soportadotable->item(pos,5)!=0) ? ui.imp_soportadotable->item(pos,5)->text() : "";
          stream << "} & {\\scriptsize " << estrella << "} & {\\scriptsize ";
          if (ui.imp_soportadotable->item(pos,6)!=NULL)
            stream << formatea_redondeado_sep(convapunto(ui.imp_soportadotable->item(pos,6)->text()).toDouble(),
                                     comadecimal,decimales);
         tbi+= convapunto(ui.imp_soportadotable->item(pos,1)->text()).toDouble();
         tcuota+=convapunto(ui.imp_soportadotable->item(pos,3)->text()).toDouble();
         ttotal+=convapunto(ui.imp_soportadotable->item(pos,4)->text()).toDouble();
         tcuota_efectiva+= ui.imp_soportadotable->item(pos,6)!=NULL ?
                           convapunto(ui.imp_soportadotable->item(pos,6)->text()).toDouble() : 0;
         stream << "} \\\\ \n " << "\\hline\n";
         // QMessageBox::information( this, tr("INFO"),ui.soportadotable->item(pos,0)->text());
         pos++;
       }
    stream << "{\\scriptsize \\textbf {";
    stream << tr("Totales:");
    stream << "}}";
    // colocamos línea de totales
    stream << " & {\\scriptsize ";
    stream << formatea_redondeado_sep(tbi,comadecimal,decimales);
    stream << "} & & ";

    // ahora viene la cuota
    stream << "{\\scriptsize ";
    stream << formatea_redondeado_sep(tcuota,comadecimal,decimales);
    stream << "} & {\\scriptsize ";

    // total
    stream << formatea_redondeado_sep(ttotal,comadecimal,decimales);
    stream << "} & & {\\scriptsize ";
    stream << formatea_redondeado_sep(tcuota_efectiva,comadecimal,decimales);
    stream << "} \\\\ \n";
    stream << "\\hline\n";


    stream << "\\end{tabular}\n";
    stream << "\\end{center}\n";
    stream << "\\begin{center}\n";
    stream << "{\\textbf {" << ui.cuotasop_imptextLabel->text() << " " <<
           formatea_redondeado_sep(convapunto(ui.cuota_implineEdit->text()).toDouble(),comadecimal,decimales) << "}}" << "\n";
    stream << "\\end{center}\n";

    stream << "\\\n";
    stream << "\\\n";



   // ----------------------------------------------------------------------------------
    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {" << tr("ADQUISICIONES INTRACOMUNITARIAS DE BIENES")
	  <<  "}}" << "\n";
    stream << "\\end{center}" << "\n";
    
     // -------------------------------------------------------------------------------------------------------
    stream << "\\begin{center}\n";
    stream << "\\begin{tabular}{|c|r|r|r|r|c|}\n";
    stream << "\\hline\n";
    stream << "{\\scriptsize " << tr("CUENTA") << "} & {\\scriptsize " << tr("Base Imponible") << 
       "} & {\\scriptsize " << tr("Tipo") << "} & {\\scriptsize " << tr("Cuota Sop.") << 
       "} & {\\scriptsize " << tr("Cuota Rep") << "} & {\\scriptsize " << tr("Inversión")<< "}  \\\\\n";
    stream << "\\hline\n";
    pos=0;
    while (pos<ui.aibtable->rowCount())
        {
          stream << "{\\scriptsize " << ui.aibtable->item(pos,0)->text() << 
		  "} & {\\scriptsize " <<
                                 formatea_redondeado_sep(convapunto(ui.aibtable->item(pos,1)->text()).toDouble(),
                                 comadecimal,decimales) << 
		  "} & {\\scriptsize ";
          stream << formatea_redondeado_sep(convapunto(ui.aibtable->item(pos,2)->text()).toDouble(),
                                 comadecimal,decimales) << "} & {\\scriptsize " << 
               formatea_redondeado_sep(convapunto(ui.aibtable->item(pos,3)->text()).toDouble(),
                                 comadecimal,decimales) << "} & {\\scriptsize ";
          stream << formatea_redondeado_sep(convapunto(ui.aibtable->item(pos,4)->text()).toDouble(),
                                 comadecimal,decimales);
          QString estrella=(ui.aibtable->item(pos,5)!=0) ? ui.aibtable->item(pos,5)->text() : "";
          stream << "} & {\\scriptsize " << estrella;
          stream << "} \\\\ \n " << "\\hline\n";
          pos++;
        }
    stream << "\\end{tabular}\n";
    stream << "\\end{center}\n";


    stream << "\\\n";
    stream << "\\\n";
// -----------------------------------------------------------------------------------

    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {" << tr("ADQUISICIONES INTRACOMUNITARIAS DE SERVICIOS")
          <<  "}}" << "\n";
    stream << "\\end{center}" << "\n";

     // -------------------------------------------------------------------------------------------------------
    stream << "\\begin{center}\n";
    stream << "\\begin{tabular}{|c|r|r|r|r|}\n";
    stream << "\\hline\n";
    stream << "{\\scriptsize " << tr("CUENTA") << "} & {\\scriptsize " << tr("Base Imponible") <<
       "} & {\\scriptsize " << tr("Tipo") << "} & {\\scriptsize " << tr("Cuota Sop.") <<
       "} & {\\scriptsize " << tr("Cuota Rep") << "}  \\\\\n";
    stream << "\\hline\n";
    pos=0;
    while (pos<ui.autofacturastable->rowCount())
        {
          stream << "{\\scriptsize " << ui.autofacturastable->item(pos,0)->text() <<
                  "} & {\\scriptsize " <<
                                 formatea_redondeado_sep(convapunto(ui.autofacturastable->item(pos,1)->text()).toDouble(),
                                 comadecimal,decimales) <<
                  "} & {\\scriptsize ";
          stream << formatea_redondeado_sep(convapunto(ui.autofacturastable->item(pos,2)->text()).toDouble(),
                                 comadecimal,decimales) << "} & {\\scriptsize " <<
               formatea_redondeado_sep(convapunto(ui.autofacturastable->item(pos,3)->text()).toDouble(),
                                 comadecimal,decimales) << "} & {\\scriptsize ";
          stream << formatea_redondeado_sep(convapunto(ui.autofacturastable->item(pos,4)->text()).toDouble(),
                                 comadecimal,decimales);
          stream << "} \\\\ \n " << "\\hline\n";
          pos++;
        }
    stream << "\\end{tabular}\n";
    stream << "\\end{center}\n";

// -----------------------------------------------------------------------------------
    stream << "\\\n";
    stream << "\\\n";

    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {" << tr("COMPRAS DE SERVICIOS EXTERIORES UE")
          <<  "}}" << "\n";
    stream << "\\end{center}" << "\n";

    stream << "\\begin{center}\n";
    stream << "\\begin{tabular}{|c|r|r|r|r|}\n";
    stream << "\\hline\n";
    stream << "{\\scriptsize " << tr("CUENTA") << "} & {\\scriptsize " << tr("Base Imponible") <<
       "} & {\\scriptsize " << tr("Tipo") << "} & {\\scriptsize " << tr("Cuota Sop.") <<
       "} & {\\scriptsize " << tr("Cuota Rep") << "}  \\\\\n";
    stream << "\\hline\n";
    pos=0;
    while (pos<ui.autofacturas_exttable->rowCount())
        {
          stream << "{\\scriptsize " << ui.autofacturas_exttable->item(pos,0)->text() <<
                  "} & {\\scriptsize " <<
                                 formatea_redondeado_sep(convapunto(ui.autofacturas_exttable->item(pos,1)->text()).toDouble(),
                                 comadecimal,decimales) <<
                  "} & {\\scriptsize ";
          stream << formatea_redondeado_sep(convapunto(ui.autofacturas_exttable->item(pos,2)->text()).toDouble(),
                                 comadecimal,decimales) << "} & {\\scriptsize " <<
               formatea_redondeado_sep(convapunto(ui.autofacturas_exttable->item(pos,3)->text()).toDouble(),
                                 comadecimal,decimales) << "} & {\\scriptsize ";
          stream << formatea_redondeado_sep(convapunto(ui.autofacturas_exttable->item(pos,4)->text()).toDouble(),
                                 comadecimal,decimales);
          stream << "} \\\\ \n " << "\\hline\n";
          pos++;
        }
    stream << "\\end{tabular}\n";
    stream << "\\end{center}\n";

// -----------------------------------------------------------------------------------
// ISP operaciones interiores

    // -----------------------------------------------------------------------------------
        stream << "\\\n";
        stream << "\\\n";

        stream << "\\begin{center}" << "\n";
        stream << "{\\Large \\textbf {" << tr("INVERSIÓN SUJETO PASIVO OPERACIONES INTERIORES")
              <<  "}}" << "\n";
        stream << "\\end{center}" << "\n";

        stream << "\\begin{center}\n";
        stream << "\\begin{tabular}{|c|r|r|r|r|}\n";
        stream << "\\hline\n";
        stream << "{\\scriptsize " << tr("CUENTA") << "} & {\\scriptsize " << tr("Base Imponible") <<
           "} & {\\scriptsize " << tr("Tipo") << "} & {\\scriptsize " << tr("Cuota Sop.") <<
           "} & {\\scriptsize " << tr("Cuota Rep") << "}  \\\\\n";
        stream << "\\hline\n";
        pos=0;
        while (pos<ui.autofacturas_interiorestable->rowCount())
            {
              stream << "{\\scriptsize " << ui.autofacturas_interiorestable->item(pos,0)->text() <<
                      "} & {\\scriptsize " <<
                                     formatea_redondeado_sep(convapunto(ui.autofacturas_interiorestable->item(pos,1)->text()).toDouble(),
                                     comadecimal,decimales) <<
                      "} & {\\scriptsize ";
              stream << formatea_redondeado_sep(convapunto(ui.autofacturas_interiorestable->item(pos,2)->text()).toDouble(),
                                     comadecimal,decimales) << "} & {\\scriptsize " <<
                   formatea_redondeado_sep(convapunto(ui.autofacturas_interiorestable->item(pos,3)->text()).toDouble(),
                                     comadecimal,decimales) << "} & {\\scriptsize ";
              stream << formatea_redondeado_sep(convapunto(ui.autofacturas_interiorestable->item(pos,4)->text()).toDouble(),
                                     comadecimal,decimales);
              stream << "} \\\\ \n " << "\\hline\n";
              pos++;
            }
        stream << "\\end{tabular}\n";
        stream << "\\end{center}\n";

    // -----------------------------------------------------------------------------------


    stream << "\\\n";
    stream << "\\\n";


    stream << "\\begin{center}\n";
    stream << "{\\Large \\textbf {" << tr("Entregas intracomunitarias de bienes:  ") << 
         formatea_redondeado_sep(convapunto(ui.eiblineEdit->text()).toDouble(),comadecimal,decimales) <<  "}}" << "\n";
    stream << "\\end{center}\n";

    stream << "\\\n";

    stream << "\\begin{center}\n";
    stream << "{\\Large \\textbf {" << tr("Exportaciones bienes y servicios:  ") <<
         formatea_redondeado_sep(convapunto(ui.exportlineEdit->text()).toDouble(),comadecimal,decimales) <<  "}}" << "\n";
    stream << "\\end{center}\n";

    stream << "\\begin{center}\n";
    stream << "{\\large \\textbf {" << tr("Facturas de operaciones interiores emitidas exentas que originan derecho a deducción:  ") <<
         formatea_redondeado_sep(convapunto(ui.exentas_deduclineEdit->text()).toDouble(),comadecimal,decimales) <<  "}}" << "\n";
    stream << "\\end{center}\n";

    stream << "\\begin{center}\n";
    stream << "{\\large \\textbf {" << tr("Facturas de operaciones emitidas exentas que NO originan derecho a deducción:  ") <<
         formatea_redondeado_sep(convapunto(ui.exentas_no_deduclineEdit->text()).toDouble(),comadecimal,decimales) <<  "}}" << "\n";
    stream << "\\end{center}\n";


     // imprimimos final del documento latex
    stream << "% FIN_CUERPO" << "\n";
    stream << "\\end{document}" << "\n";

    fichero.close();


}



void liquidacioniva::imprimir()
{
   generalatex();

   int valor=imprimelatex2(tr("liquidacion_IVA"));
   if (valor==1)
       QMessageBox::warning( this, tr("Imprimir liquidación IVA"),tr("PROBLEMAS al llamar a Latex"));
   if (valor==2)
       QMessageBox::warning( this, tr("Imprimir liquidación IVA"),
                                tr("PROBLEMAS al llamar a 'dvips'"));
   if (valor==3)
       QMessageBox::warning( this, tr("Imprimir liquidación IVA"),
                             tr("PROBLEMAS al llamar a ")+programa_imprimir());

}

void liquidacioniva::latex()
{
    generalatex();
    int valor=editalatex("liquidacion_IVA");
    if (valor==1)
        QMessageBox::warning( this, tr("LIQUIDACIÓN IVA"),tr("PROBLEMAS al llamar al editor Latex"));

}


void liquidacioniva::copia()
{
    QClipboard *cb = QApplication::clipboard();
    QString global;

    global=nombreempresa();
    global+="\n";
    global+="\n";
    global+=tr("LIQUIDACIÓN DE IVA") + "\n\n";
    global+=tr("Período de ") + ui.inicialdateEdit->date().toString("dd.MM.yyyy");
    global+=tr(" a ") + ui.finaldateEdit->date().toString("dd.MM.yyyy") + "\n\n";


    global +=  tr("IVA REPERCUTIDO") + "\n";

     // --------------------------------------------------------------------------------------------------
    global += tr("CUENTA") + "\t" + tr("Base Imponible") + "\t" +
      tr("Tipo") + "\t" + tr("Cuota") + "\t" + tr("T.R.E.") + "\t" + tr("R.E.")
      + "\t" + tr("Cuota C.Caja") + "\n";
    int pos=0;
    while (pos<ui.repercutidotable->rowCount())
        {
          global += ui.repercutidotable->item(pos,0)->text();
          global += "\t";
          global += ui.repercutidotable->item(pos,1)->text();
          global += "\t";
          global += ui.repercutidotable->item(pos,2)->text();
          global += "\t";
          if (ui.repercutidotable->item(pos,6)->text().isEmpty())
             global += ui.repercutidotable->item(pos,3)->text();
          global += "\t";
          global += ui.repercutidotable->item(pos,4)->text();
          global += "\t";
          global += ui.repercutidotable->item(pos,5)->text();
          global += "\t";
          global += ui.repercutidotable->item(pos,6)->text();
          global += "\n ";
         pos++;
       }
    global += tr("Total IVA repercutido:  ");
    global += "\t";
    global += ui.ivalineEdit->text();
    global += "\n";
    global += tr(" - Total recargo equivalencia: \t");
    global += ui.relineEdit->text();
    global += "\n";
    global += "\n";

    // --------------------------------------------------------------------------------------

    global+= tr("IVA SOPORTADO") + "\n\n";

    global+=tr("CUENTA\t") + tr("Base Imponible\t") + tr("Tipo\t") + tr("Cuota\t");
    global+=tr("Total\t") + tr("Inversión\t") + tr("Cuota efectiva") + tr("Cuota C.Caja") + "\n";
    pos=0;
    while (pos<ui.soportadotable->rowCount())
        {
          global += ui.soportadotable->item(pos,0)->text();
          global += "\t";
          global += ui.soportadotable->item(pos,1)->text();
          global += "\t";
          global += ui.soportadotable->item(pos,2)->text();
          global += "\t";
          global += ui.soportadotable->item(pos,3)->text();
          global += "\t";
          global += ui.soportadotable->item(pos,4)->text();
          global += "\t";
          QString estrella=(ui.soportadotable->item(pos,5)!=0) ? ui.soportadotable->item(pos,5)->text() : "";
          global += estrella;
          global += "\t";
          if (ui.soportadotable->item(pos,7)->text().isEmpty())
             {
              global += (ui.soportadotable->item(pos,6)!=0) ?
                    ui.soportadotable->item(pos,6)->text() : "";
             }
          global += "\t";
          global += ui.soportadotable->item(pos,7)->text();
          global += "\n ";
         // QMessageBox::information( this, tr("INFO"),ui.soportadotable->item(pos,0)->text());
         pos++;
       }
    global += "\n";
    global += ui.cuotasoptextLabel->text();
    global += "\t";
    global += ui.cuotalineEdit->text();
    global += "\n";

    global += "\n";

    //------------------------------------------------------------------------
    //------------- IVA SOPORTADO EN IMPORTACIONES

    global+= tr("IVA SOPORTADO EN IMPORTACIONES") + "\n\n";

    global+=tr("CUENTA\t") + tr("Base Imponible\t") + tr("Tipo\t") + tr("Cuota\t");
    global+=tr("Total\t") + tr("Inversión\t") + tr("Cuota efectiva") + "\n";
    pos=0;
    while (pos<ui.imp_soportadotable->rowCount())
        {
          global += ui.imp_soportadotable->item(pos,0)->text();
          global += "\t";
          global += ui.imp_soportadotable->item(pos,1)->text();
          global += "\t";
          global += ui.imp_soportadotable->item(pos,2)->text();
          global += "\t";
          global += ui.imp_soportadotable->item(pos,3)->text();
          global += "\t";
          global += ui.imp_soportadotable->item(pos,4)->text();
          global += "\t";
          QString estrella=(ui.imp_soportadotable->item(pos,5)!=0) ?
                           ui.imp_soportadotable->item(pos,5)->text() : "";
          global += estrella;
          global += "\t";
          global += (ui.imp_soportadotable->item(pos,6)!=0) ?
                    ui.imp_soportadotable->item(pos,6)->text() : "";
          global += "\n ";
         // QMessageBox::information( this, tr("INFO"),ui.soportadotable->item(pos,0)->text());
         pos++;
       }
    global += "\n";
    global += ui.cuotasop_imptextLabel->text();
    global += "\t";
    global += ui.cuota_implineEdit->text();
    global += "\n";

    global += "\n";
    //----------------------------------------------------------------------------------------------------------

    global += tr("ADQUISICIONES INTRACOMUNITARIAS DE BIENES");
    global += "\n";

     // -------------------------------------------------------------------------------------------------------
    global += tr("CUENTA") + "\t" + tr("Base Imponible") +
       "\t" + tr("Tipo") + "\t" + tr("Cuota Sop.") +
       "\t" + tr("Cuota Rep") + "\t" + tr("Inversión") + "\n";
    pos=0;
    while (pos<ui.aibtable->rowCount())
        {
          global += ui.aibtable->item(pos,0)->text();
          global += "\t";
          global += ui.aibtable->item(pos,1)->text();
          global += "\t";
          global += ui.aibtable->item(pos,2)->text();
          global += "\t";
          global += ui.aibtable->item(pos,3)->text();
          global += "\t";
          global += ui.aibtable->item(pos,4)->text();
          global += "\t";
          QString estrella=(ui.aibtable->item(pos,5)!=0) ? ui.aibtable->item(pos,5)->text() : "";
          global += estrella;
          global += "\n ";
          pos++;
        }
    global += "\n";
    global += "\n";


// -----------------------------------------------------------------------------------

    global += tr("ADQUISICIONES INTRACOMUNITARIAS DE SERVICIOS") + "\n";

     // -------------------------------------------------------------------------------------------------------
    global += tr("CUENTA") + "\t" + tr("Base Imponible") +
       "\t" + tr("Tipo") +"\t" + tr("Cuota Sop.") +
       "\t" + tr("Cuota Rep") + "\n";
    pos=0;
    while (pos<ui.autofacturastable->rowCount())
        {
          global += ui.autofacturastable->item(pos,0)->text();
          global += "\t";
          global += ui.autofacturastable->item(pos,1)->text();
          global += "\t";
          global += ui.autofacturastable->item(pos,2)->text();
          global += "\t";
          global += ui.autofacturastable->item(pos,3)->text();
          global += "\t";
          global += ui.autofacturastable->item(pos,4)->text();
          global += "\n ";
          pos++;
        }
    global += "\n";
    global += "\n";

// -----------------------------------------------------------------------------------
    global += tr("COMPRAS DE SERVICIOS EXTERIORES UE");
    global += "\n";

     // -------------------------------------------------------------------------------------------------------
    global += tr("CUENTA") + "\t" + tr("Base Imponible") +
       "\t" + tr("Tipo") + "\t" + tr("Cuota Sop.") +
       "\t" + tr("Cuota Rep") + "\n";
    pos=0;
    while (pos<ui.autofacturas_exttable->rowCount())
        {
          global +=  ui.autofacturas_exttable->item(pos,0)->text();
          global += "\t";
          global +=  ui.autofacturas_exttable->item(pos,1)->text();
          global += "\t";
          global += ui.autofacturas_exttable->item(pos,2)->text();
          global += "\t";
          global += ui.autofacturas_exttable->item(pos,3)->text();
          global += "\t";
          global += ui.autofacturas_exttable->item(pos,4)->text();
          global += "\n";
          pos++;
        }

    global += "\n";
    global += "\n";
// -----------------------------------------------------------------------------------


    // -----------------------------------------------------------------------------------
        global += tr("INVERSIÓN SUJETO PASIVO EN OPERACIONES INTERIORES");
        global += "\n";

         // -------------------------------------------------------------------------------------------------------
        global += tr("CUENTA") + "\t" + tr("Base Imponible") +
           "\t" + tr("Tipo") + "\t" + tr("Cuota Sop.") +
           "\t" + tr("Cuota Rep") + "\n";
        pos=0;
        while (pos<ui.autofacturas_interiorestable->rowCount())
            {
              global +=  ui.autofacturas_interiorestable->item(pos,0)->text();
              global += "\t";
              global +=  ui.autofacturas_interiorestable->item(pos,1)->text();
              global += "\t";
              global += ui.autofacturas_interiorestable->item(pos,2)->text();
              global += "\t";
              global += ui.autofacturas_interiorestable->item(pos,3)->text();
              global += "\t";
              global += ui.autofacturas_interiorestable->item(pos,4)->text();
              global += "\n";
              pos++;
            }

    // -----------------------------------------------------------------------------------


    global += "\n";
    global += "\n";


    // -----------------------------------------------------------------------------------
        global += tr("IVA COMPENSACIÓN EN RÉGIMEN AGRARIO/PESCA");
        global += "\n";

         // -------------------------------------------------------------------------------------------------------
        global += tr("CUENTA") + "\t" + tr("Base Imponible") +
           "\t" + tr("Tipo") + "\t" + tr("Cuota Sop.") +
           "\t" + tr("Total") + "\n";
        pos=0;
        while (pos<ui.agrariotableWidget->rowCount())
            {
              global +=  ui.agrariotableWidget->item(pos,0)->text();
              global += "\t";
              global +=  ui.agrariotableWidget->item(pos,1)->text();
              global += "\t";
              global += ui.agrariotableWidget->item(pos,2)->text();
              global += "\t";
              global += ui.agrariotableWidget->item(pos,3)->text();
              global += "\t";
              global += ui.agrariotableWidget->item(pos,4)->text();
              global += "\n";
              pos++;
            }

    // -----------------------------------------------------------------------------------


    global += "\n";
    global += "\n";


    global +=  tr("Entregas intracomunitarias de bienes:  ") + "\t" +
         ui.eiblineEdit->text();
    global += "\n";

    global += "\n";

    global +=  tr("Prestaciones intracomunitarias de servicios:  ") + "\t" +
         ui.pr_servlineEdit->text();
    global += "\n";

    global += "\n";

    global +=  tr("Exportaciones bienes y servicios:  ") + "\t" +
         ui.exportlineEdit->text();
    global += "\n";

    global += "\n";

    global +=  tr("Exentas derecho deducción:  ") + "\t" +
         ui.exentas_deduclineEdit->text();
    global += "\n";


    global += "\n";

    global +=  tr("Exentas no derecho deducción:  ") + "\t" +
         ui.exentas_no_deduclineEdit->text();
    global += "\n";


    cb->setText(global);
    QMessageBox::information( this, tr("Liquidación de IVA"),
                              tr("Se ha pasado el contenido al portapapeles") );
}


void liquidacioniva::ver_iva_caja_soportado_corrientes()
{
 ts_caja_corr->exec();
}


void liquidacioniva::ver_iva_caja_soportado_inversion()
{
 ts_caja_inv->exec();
}


void liquidacioniva::ver_iva_caja_repercutido()
{
 ts_caja_rep->exec();
}


void liquidacioniva::gen_asiento()
{
  tabla_asientos *t = new tabla_asientos(comadecimal,decimales,usuario);

  //void pasadatos1( int fila, codigocuenta, concepto, debe,
  //                 haber, documento, ci);
  // se encarga de crear todas las celdas
  QString concepto=tr("Regularización IVA");
  t->marca_regul_iva();
  int pos=0;
  int fila=0;
  while (pos<ui.repercutidotable->rowCount())
      {
        if (!existesubcuenta(ui.repercutidotable->item(pos,0)->text()))
           {
            pos++;
            continue;
           }
        double cuota_mas_re=0;
        cuota_mas_re=convapunto(ui.repercutidotable->item(pos,3)->text()).toDouble() +
                     convapunto(ui.repercutidotable->item(pos,5)->text()).toDouble();
        QString debe=ui.repercutidotable->item(pos,6)->text().isEmpty() ?
                     formateanumero(cuota_mas_re,comadecimal,decimales) :
                     ui.repercutidotable->item(pos,6)->text();
        QString conceptoad=tr(" al tipo ");
        conceptoad+=ui.repercutidotable->item(pos,2)->text();
        // hay RE ?
        if (convapunto(ui.repercutidotable->item(pos,4)->text()).toDouble()>0.0001)
           {
             conceptoad+=tr(" RE al ") ;
             conceptoad+=ui.repercutidotable->item(pos,4)->text();
           }
        if (!ui.repercutidotable->item(pos,6)->text().isEmpty())
            conceptoad+=tr(" régimen especial criterio de caja");
        if (convapunto(debe).toDouble()>0.001 || convapunto(debe).toDouble()<-0.001 )
          {
           t->pasadatos1(fila,ui.repercutidotable->item(pos,0)->text(),concepto+conceptoad,debe,"","","");
           fila++;
          }
        pos++;
       }

  pos=0;
  concepto=tr("Regularización IVA");
  while (pos<ui.soportadotable->rowCount())
      {
       if (!existesubcuenta(ui.soportadotable->item(pos,0)->text()))
         {
          pos++;
          continue;
         }
       QString haber=ui.soportadotable->item(pos,7)->text().isEmpty() ?
                      ui.soportadotable->item(pos,6)->text() :
                      ui.soportadotable->item(pos,7)->text();

       QString conceptoad=tr(" al tipo ");
       conceptoad+=ui.soportadotable->item(pos,2)->text();
       if (!ui.soportadotable->item(pos,5)->text().isEmpty())
          conceptoad+= tr(" bienes inversión");
       if (!ui.soportadotable->item(pos,7)->text().isEmpty())
          conceptoad+= tr(" régimen especial criterio de caja");
       // t->pasadatos1(fila,ui.soportadotable->item(pos,0)->text(),concepto+conceptoad,"",haber,"","");
       if (convapunto(haber).toDouble()>0.001 || convapunto(haber).toDouble()<-0.001 )
         {
          t->pasadatos1(fila,ui.soportadotable->item(pos,0)->text(),concepto+conceptoad,"",haber,"","");
          fila++;
         }
       pos++;
     }
  //------------------------------------------------------------------------
  //------------- IVA SOPORTADO EN IMPORTACIONES

  pos=0;
  concepto=tr("Regularización IVA importaciones ");
  while (pos<ui.imp_soportadotable->rowCount())
      {
      if (!existesubcuenta(ui.imp_soportadotable->item(pos,0)->text()))
        {
         pos++;
         continue;
        }
      QString haber=ui.imp_soportadotable->item(pos,6)->text();

      QString conceptoad=tr("al tipo ");
      conceptoad+=ui.imp_soportadotable->item(pos,2)->text();
      if (!ui.imp_soportadotable->item(pos,5)->text().isEmpty())
         conceptoad+= tr(" bienes inversión");
      t->pasadatos1(fila,ui.imp_soportadotable->item(pos,0)->text(),concepto+conceptoad,"",haber,"","");
      pos++; fila++;
     }

  //----------------------------------------------------------------------------------------------------------

  // "ADQUISICIONES INTRACOMUNITARIAS DE BIENES"
  pos=0;
  concepto=tr("Regularización IVA AIB");
  while (pos<ui.aibtable->rowCount())
      {
       if (!existesubcuenta(ui.aibtable->item(pos,0)->text()))
        {
         pos++;
         continue;
        }
       QString haber=ui.aibtable->item(pos,3)->text();
       QString debe=ui.aibtable->item(pos,4)->text();
       QString conceptoad;
       if (!ui.aibtable->item(pos,5)->text().isEmpty())
           conceptoad=tr(" bien inversión");
       t->pasadatos1(fila,ui.aibtable->item(pos,0)->text(),concepto+conceptoad,debe,haber,"","");
       pos++; fila++;
      }

// -----------------------------------------------------------------------------------
// "ADQUISICIONES INTRACOMUNITARIAS DE SERVICIOS"

  pos=0;
  concepto=tr("Regularización IVA Adquisiciones intracomunitarias de servicios");
  while (pos<ui.autofacturastable->rowCount())
      {
      if (!existesubcuenta(ui.autofacturastable->item(pos,0)->text()))
       {
        pos++;
        continue;
       }
      QString haber=ui.autofacturastable->item(pos,3)->text();
      QString debe=ui.autofacturastable->item(pos,4)->text();
      t->pasadatos1(fila,ui.autofacturastable->item(pos,0)->text(),concepto,debe,haber,"","");
      pos++; fila++;
      }

// -----------------------------------------------------------------------------------
// "COMPRAS DE SERVICIOS EXTERIORES UE"
  pos=0;
  concepto=tr("Regularización IVA compra servicios exteriores UE");
  while (pos<ui.autofacturas_exttable->rowCount())
      {
      if (!existesubcuenta(ui.autofacturas_exttable->item(pos,0)->text()))
       {
        pos++;
        continue;
       }
      QString haber=ui.autofacturas_exttable->item(pos,3)->text();
      QString debe=ui.autofacturas_exttable->item(pos,4)->text();
      t->pasadatos1(fila,ui.autofacturas_exttable->item(pos,0)->text(),concepto,debe,haber,"","");
      pos++; fila++;
      }

// -----------------------------------------------------------------------------------


// -----------------------------------------------------------------------------------
// "INVERSIÓN SUJETO PASIVO EN OPERACIONES INTERIORES"
      pos=0;
      concepto=tr("Regularización IVA ISP operaciones interiores");
      while (pos<ui.autofacturas_interiorestable->rowCount())
          {
          if (!existesubcuenta(ui.autofacturas_interiorestable->item(pos,0)->text()))
           {
            pos++;
            continue;
           }
          QString haber=ui.autofacturas_interiorestable->item(pos,3)->text();
          QString debe=ui.autofacturas_interiorestable->item(pos,4)->text();
          t->pasadatos1(fila,ui.autofacturas_interiorestable->item(pos,0)->text(),concepto,debe,haber,"","");
          pos++; fila++;
          }

  // -----------------------------------------------------------------------------------
  //"IVA COMPENSACIÓN EN RÉGIMEN AGRARIO/PESCA"
      concepto=tr("Regularización IVA compensación en régimen agrario/pesca");
      pos=0;
      while (pos<ui.agrariotableWidget->rowCount())
          {
          if (!existesubcuenta(ui.agrariotableWidget->item(pos,0)->text()))
           {
            pos++;
            continue;
           }
          QString haber=ui.agrariotableWidget->item(pos,3)->text();
          QString debe;
          t->pasadatos1(fila,ui.agrariotableWidget->item(pos,0)->text(),concepto,debe,haber,"","");
          pos++; fila++;
          }

  // -----------------------------------------------------------------------------------
t->pasafechaasiento(ui.finaldateEdit->date());
t->chequeatotales();
t->exec();
}
