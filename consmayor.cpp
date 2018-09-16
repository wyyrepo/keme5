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

#include "consmayor.h"
#include "funciones.h"
#include "basedatos.h"
#include "buscasubcuenta.h"
#include "tabla_asientos.h"
#include "cuadimprimayor.h"
#include "editarasiento.h"
#include "datos_accesorios.h"
#include "privilegios.h"
#include "consultavencipase.h"
#include "introfecha.h"
#include "procesavencimiento.h"
#include "acumuladosmes.h"
#include "asigna_fichdoc.h"
#include <QMessageBox>

consmayor::consmayor(bool concomadecimal, bool condecimales, QString qusuario) : QDialog() {
   ui.setupUi(this);

  QDate fechaactual;
  QString cadfechaactual;
  comadecimal=concomadecimal;
  decimales=condecimales;
  cadfechaactual=fechaactual.currentDate().toString("yyyy-MM-dd");

  ui.inicialdateEdit->setDate( basedatos::instancia()->selectAperturaejerciciosaperturacierre( cadfechaactual, cadfechaactual ) );
  ui.finaldateEdit->setDate( basedatos::instancia()->selectCierreejerciciosaperturacierre( cadfechaactual, cadfechaactual ) );

 QStringList columnas;
 columnas << tr("CUENTA") << tr("FECHA") << tr("ASTO.") << tr("CONCEPTO");
 columnas << tr("DEBE") << tr("HABER") << tr("SALDO");
 columnas << tr("DOCUMENTO") << tr("DIARIO") << tr("CI") << tr("APUNTE");

 ui.mayortable->setColumnCount(11);

 ui.mayortable->hideColumn(0); // el intervalo de cuentas está desactivado por defecto

 ui.mayortable->setEditTriggers ( QAbstractItemView::NoEditTriggers );

 ui.mayortable->setHorizontalHeaderLabels(columnas);

 ui.mayortable->setColumnWidth(1,80);
 ui.mayortable->setColumnWidth(2,60);
 ui.mayortable->setColumnWidth(3,300);
 ui.mayortable->setColumnWidth(4,80);
 ui.mayortable->setColumnWidth(5,80);
 ui.mayortable->setColumnWidth(6,80);
 ui.mayortable->setColumnWidth(7,125);
 ui.mayortable->setColumnWidth(8,80);
 ui.mayortable->setColumnWidth(9,125);

 if (!conanalitica()) ui.mayortable->hideColumn(8);

 connect(ui.buscapushButton,SIGNAL(clicked()),SLOT(botonsubcuentapulsado()));
 connect(ui.refrescarpushButton,SIGNAL(clicked()),SLOT(refrescar()));
 connect(ui.subcuentalineEdit,SIGNAL(editingFinished()),SLOT(finedicsubcuenta()));
 connect(ui.previapushButton,SIGNAL(clicked()),SLOT(subcuentaprevia()));
 connect(ui.siguientepushButton,SIGNAL(clicked()),SLOT(subcuentasiguiente()));
 connect(ui.imprimirpushButton,SIGNAL(clicked()),SLOT(imprimir()));
 connect(ui.subcuentalineEdit,SIGNAL(textChanged(QString)),SLOT(subcuentacambiada()));
 connect(ui.editarpushButton,SIGNAL(clicked()),SLOT(editarpulsado()));
 connect(ui.xmlpushButton,SIGNAL(clicked()),SLOT(genxml()));

 connect(ui.origencheckBox,SIGNAL(stateChanged(int)), SLOT(origenpercambiado()));
 connect(ui.fincheckBox,SIGNAL(stateChanged(int)), SLOT(finalpercambiado()));

 connect(ui.intervalogroupBox,SIGNAL(toggled(bool)),this,SLOT(intervalogroupcambiado()));

 connect(ui.copiarpushButton,SIGNAL(clicked()),SLOT(copiar()));
 connect(ui.borrarpushButton,SIGNAL(clicked()),SLOT(borraasiento()));

 connect(ui.visdocpushButton,SIGNAL(clicked()),SLOT(visdoc()));
 connect(ui.texpushButton,SIGNAL(clicked()),SLOT(latex()));
 connect(ui.consultarpushButton,SIGNAL(clicked()),SLOT(consultaasiento()));

 connect(ui.datospushButton,SIGNAL(clicked()),SLOT(datoscuenta()));

 connect(ui.vtospushButton,SIGNAL(clicked()),SLOT(vencimientos()));
 connect(ui.nuevopushButton,SIGNAL(clicked()),SLOT(nuevo_asiento()));

 connect(ui.procvtopushButton,SIGNAL(clicked()),SLOT(procesa_vencimiento()));

 connect(ui.acumpushButton,SIGNAL(clicked()),SLOT(acumuladoscta()));

 connect(ui.asignafichdocpushButton,SIGNAL(clicked()),SLOT(asigna_ficherodocumento()));


 usuario=qusuario;

 QString privileg=basedatos::instancia()->privilegios(qusuario);

 if (privileg[datos_accesorios_cta]=='0') ui.datospushButton->setEnabled(false);
 if (privileg[edi_asiento]=='0') ui.editarpushButton->setEnabled(false);
 if (privileg[borrar_asiento]=='0') ui.borrarpushButton->setEnabled(false);
 if (privileg[vencimientos_asignar]=='0') ui.vtospushButton->setEnabled(false);

 if (privileg[vencimientos_procesar]=='0') ui.procvtopushButton->setEnabled(false);

 if (privileg[nuev_asiento]=='0') ui.nuevopushButton->setEnabled(false);

 if (privileg[asigna_fichero]=='0') ui.asignafichdocpushButton->setEnabled(false);


 ui.fotolabel->setFixedHeight(0);
 ui.fotolabel->setFixedWidth(0);

#ifdef NOEDITTEX
 ui.texpushButton->hide();
#endif


}



void consmayor::botonsubcuentapulsado()
{
    QString cadena2;
    buscasubcuenta *labusqueda=new buscasubcuenta(ui.subcuentalineEdit->text());
    int cod=labusqueda->exec();
    cadena2=labusqueda->seleccioncuenta();
    if (cod==QDialog::Accepted && existesubcuenta(cadena2))
      ui.subcuentalineEdit->setText(cadena2);
       else ui.subcuentalineEdit->setText("");
    delete labusqueda;

}



void consmayor::cargadatos()
{
   bool intervalo=false;
   QString condicion;
   if (ui.intervalogroupBox->isChecked()) intervalo=true;
   if (!intervalo)
      {
       if (!existesubcuenta(ui.subcuentalineEdit->text())) return;

       if (escuentadegasto(ui.subcuentalineEdit->text()) &&
           escuentadeingreso(ui.subcuentalineEdit->text()) &&
           (conanalitica() || conanalitica_parc()))
         ui.mayortable->showColumn(9);
        else ui.mayortable->hideColumn(9);

       condicion="cuenta='";
       condicion+=ui.subcuentalineEdit->text();
       condicion+="'";
       QString imagen=basedatos::instancia()->fotocuenta (ui.subcuentalineEdit->text());
       QPixmap foto;
       if (imagen.length()>0)
          {
           QByteArray byteshexa;
           byteshexa.append ( imagen );
           QByteArray bytes;
           bytes=bytes.fromHex ( byteshexa );
           foto.loadFromData ( bytes, "PNG");
           ui.fotolabel->setFixedHeight(100);
           ui.fotolabel->setFixedWidth(90);
          }
           else
               {
                ui.fotolabel->setFixedHeight(0);
                ui.fotolabel->setFixedWidth(0);
               }
       ui.fotolabel->setPixmap(foto);
      }
      else
           {
             if (conanalitica()) ui.mayortable->showColumn(9);
                else ui.mayortable->hideColumn(9);
             if (ui.iniciallineEdit->text().length()==0 || ui.finallineEdit->text().length()==0)
                {
                 QMessageBox::warning( this, tr("Consulta de mayor"),
	 	 tr("ERROR: se debe de suministrar información correcta para el intervalo de cuentas"));
                 return;
                }
             condicion+="cuenta>='";
             condicion+=ui.iniciallineEdit->text();
             condicion+="' and ";
             condicion+="cuenta<='";
             condicion+=ui.finallineEdit->text();
             condicion+="'";
             ui.nombrelabel->setText(tr("Intervalo: ")+ui.iniciallineEdit->text()+
                                      " - "+ui.finallineEdit->text());
           }

    qlonglong num = basedatos::instancia()->selectCountasientodiariofechascondicion( ui.inicialdateEdit->date() ,ui.finaldateEdit->date() , condicion );
    if (num > 15000)
    {
	    switch( QMessageBox::warning( this, tr("Consultas de Mayor"),
            tr("La consulta excede de 15000 registros,\n"
            "¿ desea cargarlos de todas formas ?"),
            tr("&Sí"), tr("&No"), 0, 0,1 ) ) 
	    {
            case 0: // proseguimos.
                break;
            case 1: // Cancelamos la carga
                return;
            }
	}
   ui.mayortable->setRowCount(num);

   QSqlQuery query = basedatos::instancia()->select9Diariofechascondicionorderfechapase(
           ui.inicialdateEdit->date() , ui.finaldateEdit->date() , condicion );
   int fila=0;
    if (query.isActive())
    while ( query.next() ) {
        QTableWidgetItem *newItemcta = new QTableWidgetItem(query.value(0).toString());
        ui.mayortable->setItem(fila,0,newItemcta);
        QTableWidgetItem *newItem = new QTableWidgetItem(query.value(1).toDate().toString("dd.MM.yyyy"));
        ui.mayortable->setItem(fila,1,newItem);
        QTableWidgetItem *newItemx = new QTableWidgetItem(query.value(2).toString());
        newItemx->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
        ui.mayortable->setItem(fila,2,newItemx);  // asiento
        QTableWidgetItem *newItemp = new QTableWidgetItem(query.value(3).toString());
        ui.mayortable->setItem(fila,3,newItemp);  // concepto
        double valdebe=query.value(4).toDouble();

        // ***************************************************************************************
        // ***************************************************************************************
        if (valdebe>0.0001 || valdebe<-0.0001)
          {
           QTableWidgetItem *newItem = new QTableWidgetItem(formateanumero(valdebe,comadecimal,decimales));
           newItem->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
           ui.mayortable->setItem(fila,4,newItem);  // debe
          }
          else 
               {
                QTableWidgetItem *newItempp = new QTableWidgetItem("");
                ui.mayortable->setItem(fila,4,newItempp);
               }
        double valhaber=query.value(5).toDouble();
        if (valhaber>0.0001 || valhaber<-0.0001)
          {
           QTableWidgetItem *newItemjj = new QTableWidgetItem(formateanumero(valhaber,comadecimal,decimales));
           newItemjj->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
           ui.mayortable->setItem(fila,5,newItemjj);  // haber
          }
          else 
               {
                QTableWidgetItem *newItemkk = new QTableWidgetItem("");
                ui.mayortable->setItem(fila,5,newItemkk);
               }
        QTableWidgetItem *newItemj = new QTableWidgetItem(query.value(6).toString());
        ui.mayortable->setItem(fila,7,newItemj); // documento
        QTableWidgetItem *newItem2 = new QTableWidgetItem(query.value(7).toString());
        ui.mayortable->setItem(fila,8,newItem2); // diario
        QTableWidgetItem *newItem3 = new QTableWidgetItem(query.value(8).toString());
        ui.mayortable->setItem(fila,9,newItem3); // ci
        QTableWidgetItem *newItempase = new QTableWidgetItem(query.value(9).toString());
        newItempase->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
        ui.mayortable->setItem(fila,10,newItempase); // apunte
        fila++;
    }
  // calculamos acumulado de saldo inicial
    // primero observamos si el ejercicio anterior está cerrado
    QString ejercicio=ejerciciodelafecha(ui.inicialdateEdit->date());
    QDate fechainiejercicio=inicioejercicio(ejercicio);
    QDate fechaejercicioanterior=fechainiejercicio.addDays(-1);
    QString ejercicioanterior=ejerciciodelafecha(fechaejercicioanterior);
    if (ejerciciocerrado(ejercicioanterior) || escuentadegasto(ui.subcuentalineEdit->text()) || 
	   escuentadeingreso(ui.subcuentalineEdit->text()))
       {
        query = basedatos::instancia()->selectSumdebesumhaberdiariofechascondicion(
                 fechainiejercicio, ui.inicialdateEdit->date(), condicion);
       }
       else query = basedatos::instancia()->selectSumdebesumhaberdiariofechacondicion(
                    ui.inicialdateEdit->date(),condicion);
   double saldoin=0;
   double sumadebe=0;
   double sumahaber=0;
   if ( (query.isActive()) && (query.first()) )
      {
        saldoin=query.value(0).toDouble()-query.value(1).toDouble();
        // sumadebe=query.value(0).toDouble();
        // sumahaber=query.value(1).toDouble();
        if (saldoin>0) sumadebe=saldoin;
        if (saldoin<0) sumahaber=saldoin*-1;
      }
    fila=0;
    while (fila<ui.mayortable->rowCount())
       {
          if (ui.mayortable->item(fila,4)->text().length()>0) 
	{
              sumadebe+=convapunto(ui.mayortable->item(fila,4)->text()).toDouble();
              saldoin+=convapunto(ui.mayortable->item(fila,4)->text()).toDouble();
	}
          if (ui.mayortable->item(fila,5)->text().length()>0)
	  {
              sumahaber+=convapunto(ui.mayortable->item(fila,5)->text()).toDouble();
              saldoin-=convapunto(ui.mayortable->item(fila,5)->text()).toDouble();
	  }
        QTableWidgetItem *newItemx = new QTableWidgetItem(formateanumero(saldoin,comadecimal,decimales));
        newItemx->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
        ui.mayortable->setItem(fila,6,newItemx); // saldo
        fila++;
       }
   ui.debelineEdit->setText(formateanumero(sumadebe,comadecimal,decimales));
   ui.haberlineEdit->setText(formateanumero(sumahaber,comadecimal,decimales));
   ui.saldolineEdit->setText(formateanumero(saldoin,comadecimal,decimales));
   ui.mayortable->resizeColumnToContents(0);
   ui.mayortable->resizeColumnToContents(1);
   ui.mayortable->resizeColumnToContents(2);
   ui.mayortable->resizeColumnToContents(4);
   ui.mayortable->resizeColumnToContents(5);
   ui.mayortable->resizeColumnToContents(6);
}


void consmayor::refrescar()
{
 QPixmap foto;
 ui.fotolabel->setPixmap(foto);

   if (ui.intervalogroupBox->isChecked()) 
      {
       cargadatos();
       ui.fotolabel->setFixedHeight(0);
       ui.fotolabel->setFixedWidth(0);
       return;
      }

   ctaexpandepunto();
    if (ejerciciodelafecha(ui.inicialdateEdit->date())!=ejerciciodelafecha(ui.finaldateEdit->date())
	&&(escuentadegasto(ui.subcuentalineEdit->text()) || 
	   escuentadeingreso(ui.subcuentalineEdit->text())))
	{
	   QMessageBox::warning( this, tr("Consultas de Mayor"),
				 tr("En consultas de cuentas de ingresos/gastos\n"
				      "las fechas han de pertenecer al mismo período"));
	   QString ejercicio=ejerciciodelafecha(ui.inicialdateEdit->date());
	   // QMessageBox::warning( this, tr("Consultas de Mayor"),ejercicio);
	   if (ejercicio.length()!=0) ui.finaldateEdit->setDate(finejercicio(ejercicio));
	      else ui.finaldateEdit->setDate(ui.inicialdateEdit->date());
                }

   if ( existesubcuenta(ui.subcuentalineEdit->text())) {
     QString cadena="Consultas de Mayor: ";
     cadena+=descripcioncuenta(ui.subcuentalineEdit->text());
     cargadatos();
     setWindowTitle(cadena);
     ui.nombrelabel->setText(descripcioncuenta(ui.subcuentalineEdit->text()).left(35));
   }
}


void consmayor::ctaexpandepunto()
{
  ui.subcuentalineEdit->setText(expandepunto(ui.subcuentalineEdit->text(),anchocuentas()));
}



void consmayor::finedicsubcuenta()
{
 ctaexpandepunto();
 if (ui.subcuentalineEdit->text().length()<anchocuentas() && cod_longitud_variable()) botonsubcuentapulsado();
}


void consmayor::pasadatos( QString qsubcuenta, QDate qfecha )
{
   ui.subcuentalineEdit->setText(qsubcuenta);
   QString ejercicio=ejerciciodelafecha(qfecha);
   QDate fechaini=inicioejercicio(ejercicio);
   QDate fechafin=finejercicio(ejercicio);
   ui.inicialdateEdit->setDate(fechaini);
   ui.finaldateEdit->setDate(fechafin);
   QString cadena="Consultas de Mayor: ";
   cadena+=descripcioncuenta(qsubcuenta);
   setWindowTitle(cadena);
   ui.nombrelabel->setText(descripcioncuenta(ui.subcuentalineEdit->text()).left(35));
   if (existesubcuenta(ui.subcuentalineEdit->text())) refrescar();
}


void consmayor::subcuentaprevia()
{
    QString cadena=subcuentaanterior(ui.subcuentalineEdit->text());
    if (cadena=="") return;
    ui.subcuentalineEdit->setText(cadena);
    refrescar();
}


void consmayor::subcuentasiguiente()
{
    QString cadena=subcuentaposterior(ui.subcuentalineEdit->text());
    if (cadena=="") return;
    ui.subcuentalineEdit->setText(cadena);
    refrescar();
}


void consmayor::imprimir()
{
  if (ui.intervalogroupBox->isChecked())
     {
       latexintervalo();
       int valor=imprimelatex2(tr("mayor_int"));
       if (valor==1)
         QMessageBox::warning( this, tr("Imprimir plan"),tr("PROBLEMAS al llamar a Latex"));
       if (valor==2)
         QMessageBox::warning( this, tr("Imprimir plan"),
                                tr("PROBLEMAS al llamar a 'dvips'"));
       if (valor==3)
         QMessageBox::warning( this, tr("Imprimir plan"),
                             tr("PROBLEMAS al llamar a ")+programa_imprimir());
       return;
     }

  cuadimprimayor *cuadimpri=new cuadimprimayor(comadecimal,decimales);
  cuadimpri->generalatexmayor( ui.subcuentalineEdit->text(),ui.subcuentalineEdit->text(),
 			       ui.inicialdateEdit->date(), ui.finaldateEdit->date() );
  delete cuadimpri;

   int valor=imprimelatex(tr("mayor"));
   if (valor==1)
       QMessageBox::warning( this, tr("IMPRIMIR MAYOR"),tr("PROBLEMAS al llamar a Latex"));
   if (valor==2)
       QMessageBox::warning( this, tr("IMPRIMIR MAYOR"),
                                tr("PROBLEMAS al llamar a 'dvips'"));
   if (valor==3)
       QMessageBox::warning( this, tr("IMPRIMIR MAYOR"),
                             tr("PROBLEMAS al llamar a ")+programa_imprimir());

}


void consmayor::subcuentacambiada()
{
  if (existesubcuenta(ui.subcuentalineEdit->text()))
      refrescar();
}




void consmayor::editarpulsado()
{
    QString asiento;

    if (ui.mayortable->currentItem()==0) return;
    int dia=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().left(2).toInt();
    int mes=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().mid(3,2).toInt();
    int anyo=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().right(4).toInt();
    QDate fecha(anyo,mes,dia);
    asiento=ui.mayortable->item(ui.mayortable->currentRow(),2)->text();
       
    QString ejercicio=ejerciciodelafecha(fecha);
    QString qdiario;
    qdiario=diariodeasiento(asiento,ejercicio);
    
    if (qdiario==diario_apertura() || qdiario==diario_cierre() || qdiario==diario_regularizacion())
       {
         QMessageBox::warning( this, tr("Consultas de Mayor"),
			       tr("ERROR: No se pueden editar asientos del diario de apertura, "
				  "regularización o cierre\n"));
         return;
       }

    if (!existeasiento(asiento,ejercicio))
       {
         QMessageBox::warning( this, tr("Consultas de Mayor"),
			       tr("ERROR: El asiento seleccionado ya no existe"));
         return;
       }

    editarasiento(asiento,usuario,ejercicio);

    refrescar();

}


void consmayor::origenpercambiado()
{
 if (ui.origencheckBox->isChecked())
     {
      ui.inicialdateEdit->setDate(inicioejercicio(ejerciciodelafecha(ui.inicialdateEdit->date())));
      ui.inicialdateEdit->setEnabled(false);
     }
  else
      ui.inicialdateEdit->setEnabled(true);
}

void consmayor::finalpercambiado()
{
 if (ui.fincheckBox->isChecked())
   {
    ui.finaldateEdit->setDate(finejercicio(ejerciciodelafecha(ui.inicialdateEdit->date())));
    ui.finaldateEdit->setEnabled(false);
   }
 else
    ui.finaldateEdit->setEnabled(true);
}


void consmayor::intervalogroupcambiado()
{
  if (ui.intervalogroupBox->isChecked())
     {
      ui.nombrelabel->clear();
      ui.subcuentalineEdit->setEnabled(false);
      ui.subcuentalineEdit->clear();
      ui.buscapushButton->setEnabled(false);
      ui.previapushButton->setEnabled(false);
      ui.siguientepushButton->setEnabled(false);
      QString cadena="Consultas de Mayor";
      setWindowTitle(cadena);
      ui.mayortable->showColumn(0);
     }
   else
       {
        ui.nombrelabel->clear();
        ui.subcuentalineEdit->setEnabled(true);
        ui.buscapushButton->setEnabled(true);
        ui.previapushButton->setEnabled(true);
        ui.siguientepushButton->setEnabled(true);
        ui.iniciallineEdit->clear();
        ui.finallineEdit->clear();
        QString cadena="Consultas de Mayor";
        setWindowTitle(cadena);
        ui.mayortable->hideColumn(0);
       }
}


void consmayor::latexintervalo()
{
   QString qfichero=dirtrabajo();
   qfichero.append(QDir::separator());
   qfichero=qfichero+tr("mayor_int.tex");
   // QMessageBox::warning( this, tr("Estados Contables"),qfichero);
   QString pasa;
   if (eswindows()) pasa=QFile::encodeName(qfichero);
       else pasa=qfichero;
   QFile fichero(pasa);

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
    stream << "{\\Large \\textbf {" << tr("MAYOR ") << filtracad(ui.nombrelabel->text()) <<  "}}" << "\n";
    stream << "\\end{center}" << "\n";

    // cuerpo de listado (tabla)
    stream << "\\begin{center}\n";
    stream << "\\begin{longtable}{|l|l|r|p{6cm}|r|r|r|}\n";
    stream << "\\hline\n";
    QDate fechainicial=ui.inicialdateEdit->date();
    QDate fechafinal=ui.finaldateEdit->date();
    stream << "\\multicolumn{7}{|c|} {\\textbf{";
    stream << tr("Extracto de ") << fechainicial.toString("dd.MM.yyyy") <<
                         tr(" a ") << fechafinal.toString("dd.MM.yyyy");
    stream <<  "}} \\\\";
    stream << "\\hline" << "\n";

    stream << "{\\scriptsize {" << tr("CUENTA") << "}} & {\\scriptsize {" << tr("FECHA") << 
             "}} & {\\scriptsize {" << tr("ASTO.") << "}} & {\\scriptsize {" <<
            tr("CONCEPTO") << "}} & {\\scriptsize {" << tr("DEBE") << "}} & {\\scriptsize {" << tr("HABER") << 
            "}} & {\\scriptsize {" << tr("SALDO") << "}} \\\\\n";
    stream << "\\hline\n";
    stream << "\\endfirsthead";
    // --------------------------------------------------------------------------------------------------
    stream << "\\hline" << "\n";
    stream << "{\\scriptsize {" << tr("CUENTA") << "}} & {\\scriptsize {" << tr("FECHA") << 
               "}} & {\\scriptsize {" << tr("ASTO.") << "}} & {\\scriptsize {" <<
             tr("CONCEPTO") << "}} & {\\scriptsize {" << tr("DEBE") << "}} & {\\scriptsize {" << tr("HABER") << 
             "}} & {\\scriptsize {" << tr("SALDO") << "}} \\\\\n";
    stream << "\\hline\n";
    stream << "\\endhead";
    // --------------------------------------------------------------------------------------------------

    double importe=0;
    int linactu=0;
    while (linactu<ui.mayortable->rowCount())
          {
           stream << "{\\scriptsize " << filtracad(ui.mayortable->item(linactu,0)->text()) << "} & {\\scriptsize ";
           stream << filtracad(ui.mayortable->item(linactu,1)->text()) << "} & {\\scriptsize ";
           stream << filtracad(ui.mayortable->item(linactu,2)->text()) << "} & {\\scriptsize ";
           stream << filtracad(ui.mayortable->item(linactu,3)->text()) << "} & {\\scriptsize ";
           importe=convapunto(ui.mayortable->item(linactu,4)->text()).toDouble();
           stream << formateanumerosep(importe,comadecimal,decimales) << "} & {\\scriptsize ";
           importe=convapunto(ui.mayortable->item(linactu,5)->text()).toDouble();
           stream << formateanumerosep(importe,comadecimal,decimales) << "} & {\\scriptsize ";
           importe=convapunto(ui.mayortable->item(linactu,6)->text()).toDouble();
           stream << formateanumerosep(importe,comadecimal,decimales) << "} ";
           stream << " \\\\ \n ";
           stream << "\\hline\n";
           linactu++;
          }
      importe=convapunto(ui.debelineEdit->text()).toDouble();
      stream  << "\\multicolumn{4}{|r|}{" << tr("Sumas ...") << "} & {\\scriptsize " <<
              formateanumerosep(importe,comadecimal,decimales);
      importe=convapunto(ui.haberlineEdit->text()).toDouble();
      stream << "} & {\\scriptsize " << formateanumerosep(importe,comadecimal,decimales) << "} & \\\\";
      stream << "\\hline\n";
      // final de la tabla
      stream << "\\end{longtable}\n";
      stream << "\\end{center}\n";

    // imprimimos final del documento latex
    stream << "% FIN_CUERPO" << "\n";
    stream << "\\end{document}" << "\n";

    fichero.close();


}


void consmayor::copiar()
{
   QClipboard *cb = QApplication::clipboard();
   QString global;

   global=nombreempresa();
   global+="\n";
   global+=ui.nombrelabel->text();
   global+="\n";
   global+=tr("INICIAL:");
   global+="\t";
   global+=ui.inicialdateEdit->date().toString("dd/MM/yyyy");
   global+="\n";
   global+=tr("FINAL:");
   global+="\t";
   global+=ui.finaldateEdit->date().toString("dd/MM/yyyy");
   global+="\n\n";
   if (ui.intervalogroupBox->isChecked())
      {
       global+=tr("CUENTA") +"\t";
      }
   global+=tr("FECHA") +"\t"+ tr("ASTO.") +"\t"+ tr("CONCEPTO") +
           "\t"+ tr("DEBE") +"\t"+ tr("HABER") +"\t"+ tr("SALDO")+
           "\t"+ tr("DOCUMENTO")+"\t"+ tr("DIARIO");
   if (conanalitica()) global+="\t"+ tr("CI");

   global+="\n";

    int linactu=0;
    while (linactu<ui.mayortable->rowCount())
          {
           if (ui.intervalogroupBox->isChecked())
              {
                global+=ui.mayortable->item(linactu,0)->text();
                global+="\t";
              }
           global+=ui.mayortable->item(linactu,1)->text();
           global+="\t";
           global+=ui.mayortable->item(linactu,2)->text();
           global+="\t";
           global+=ui.mayortable->item(linactu,3)->text();
           global+="\t";
           global+=ui.mayortable->item(linactu,4)->text();
           global+="\t";
           global+=ui.mayortable->item(linactu,5)->text();
           global+="\t";
           global+=ui.mayortable->item(linactu,6)->text();
           global+="\t";
           global+=ui.mayortable->item(linactu,7)->text();
           global+="\t";
           global+=ui.mayortable->item(linactu,8)->text();
           if (conanalitica())
              {
               global+="\t";
               global+=ui.mayortable->item(linactu,9)->text();
              }
           global+="\n";
           linactu++;
          }

   cb->setText(global);
   QMessageBox::information( this, tr("Consulta de mayor"),
                             tr("Se ha pasado el contenido al portapapeles") );

}



void consmayor::genxml()
{
  if (ui.intervalogroupBox->isChecked())
     {
       xmlintervalo();
       QMessageBox::information( this, tr("IMPRIMIR MAYOR"),
                                tr("El fichero XML se ha generado en el directorio de trabajo"));
       return;
     }

  // cuadimprimayor *cuadimpri=new cuadimprimayor(comadecimal,decimales);
  cuadimprimayor cuadimpri(comadecimal,decimales);
  cuadimpri.generaxmlmayor( ui.subcuentalineEdit->text(),ui.subcuentalineEdit->text(),
                               ui.inicialdateEdit->date(), ui.finaldateEdit->date() );
  //delete cuadimpri;

  QMessageBox::information( this, tr("IMPRIMIR MAYOR"),
                                tr("El fichero XML se ha generado en el directorio de trabajo"));

}



void consmayor::xmlintervalo()
{
 QDomDocument doc("Mayor");
 QDomElement root = doc.createElement("Mayor");
 doc.appendChild(root);

 QDomElement tag = doc.createElement("Cabecera");
 root.appendChild(tag);

 addElementoTextoDom(doc,tag,"NombreEmpresa",filtracadxml(nombreempresa()));
 addElementoTextoDom(doc,tag,"Seleccion",filtracadxml(ui.nombrelabel->text()));

 QDate fechainicial=ui.inicialdateEdit->date();
 QDate fechafinal=ui.finaldateEdit->date();
 addElementoTextoDom(doc,tag,"FechaInicial",filtracadxml(fechainicial.toString("yyyy/MM/dd")));
 addElementoTextoDom(doc,tag,"FechaFinal",filtracadxml(fechafinal.toString("yyyy/MM/dd")));

 QDomElement tag2 = doc.createElement("Detalle");
 root.appendChild(tag2);

 double importe=0;
 int linactu=0;
 // sacamos línea de saldo inicial a través de la primera línea de la cuenta
 double saldoinicial=convapunto(ui.mayortable->item(linactu,6)->text()).toDouble()
                     -convapunto(ui.mayortable->item(linactu,4)->text()).toDouble()
                     +convapunto(ui.mayortable->item(linactu,5)->text()).toDouble();
 if (saldoinicial>0.0001 || saldoinicial<-0.0001)
 {
   QDomElement tag3 = doc.createElement("Apunte");
   tag2.appendChild(tag3);
   addElementoTextoDom(doc,tag3,"Cuenta","");
   addElementoTextoDom(doc,tag3,"Fecha",filtracadxml(fechainicial.toString("yyyy/MM/dd")));
   addElementoTextoDom(doc,tag3,"Asiento","");
   addElementoTextoDom(doc,tag3,"Concepto",tr("Saldo inicial"));
   if (saldoinicial>0)
     addElementoTextoDom(doc,tag3,"Debe",filtracadxml(formateanumero(saldoinicial,comadecimal,decimales)));
   else
     addElementoTextoDom(doc,tag3,"Debe","");
   if (saldoinicial<0)
     addElementoTextoDom(doc,tag3,"Haber",filtracadxml(formateanumero(saldoinicial,comadecimal,decimales)));
   else
    addElementoTextoDom(doc,tag3,"Haber","");
   addElementoTextoDom(doc,tag3,"Saldo",filtracadxml(formateanumero(saldoinicial,comadecimal,decimales)));
}
 while (linactu<ui.mayortable->rowCount())
          {
           QDomElement tag3 = doc.createElement("Apunte");
           tag2.appendChild(tag3);
           addElementoTextoDom(doc,tag3,"Cuenta",filtracadxml(ui.mayortable->item(linactu,0)->text()));
           // corregir aquí fecha
           QDate fecha=fecha.fromString(ui.mayortable->item(linactu,1)->text(),"dd.MM.yyyy");
           addElementoTextoDom(doc,tag3,"Fecha",filtracadxml(fecha.toString("yyyy/MM/dd")));
           addElementoTextoDom(doc,tag3,"Asiento",filtracadxml(ui.mayortable->item(linactu,2)->text()));
           addElementoTextoDom(doc,tag3,"Concepto",filtracadxml(ui.mayortable->item(linactu,3)->text()));

           importe=convapunto(ui.mayortable->item(linactu,4)->text()).toDouble();
           addElementoTextoDom(doc,tag3,"Debe",filtracadxml(formateanumero(importe,comadecimal,decimales)));

           importe=convapunto(ui.mayortable->item(linactu,5)->text()).toDouble();
           addElementoTextoDom(doc,tag3,"Haber",filtracadxml(formateanumero(importe,comadecimal,decimales)));

           importe=convapunto(ui.mayortable->item(linactu,6)->text()).toDouble();
           addElementoTextoDom(doc,tag3,"Saldo",filtracadxml(formateanumero(importe,comadecimal,decimales)));

           linactu++;
          }

    QString xml = doc.toString();

   QString qfichero=dirtrabajo();
   qfichero.append(QDir::separator());
   qfichero=qfichero+tr("mayor_int.xml");
   // QMessageBox::warning( this, tr("Estados Contables"),qfichero);
   QString pasa;
   if (eswindows()) pasa=QFile::encodeName(qfichero);
       else pasa=qfichero;
   QFile fichero(pasa);
   if (! fichero.open( QIODevice::WriteOnly ) ) return;
   QTextStream stream( &fichero );
   stream.setCodec("UTF-8");

   stream << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
  // bssstream << "<?xml-stylesheet type=\"text/xsl\" href=\"plan2html.xslt\"?>\n";

  // ------------------------------------------------------------------------------------
  stream << xml;

  fichero.close();

}

void consmayor::borraasiento()
{
    QString asiento;

    if (ui.mayortable->currentItem()==0) return;
    int dia=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().left(2).toInt();
    int mes=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().mid(3,2).toInt();
    int anyo=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().right(4).toInt();
    QDate fecha(anyo,mes,dia);
    QString ejercicio=ejerciciodelafecha(fecha);
     asiento=ui.mayortable->item(ui.mayortable->currentRow(),2)->text();
     borrarasientofunc(asiento,ejercicio);
    refrescar();
}


void consmayor::visdoc()
{
  if (ui.mayortable->currentItem()==0) return;

  QString asiento;

  int dia=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().left(2).toInt();
  int mes=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().mid(3,2).toInt();
  int anyo=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().right(4).toInt();
  QDate fecha(anyo,mes,dia);
  QString ejercicio=ejerciciodelafecha(fecha);
  asiento=ui.mayortable->item(ui.mayortable->currentRow(),2)->text();

  QString fichdoc=basedatos::instancia()->copia_docdiario(asiento,ejercicio);

  if (fichdoc.isEmpty())
  {
    QString qapunte=ui.mayortable->item(ui.mayortable->currentRow(),10)->text();
    QString serie, numero;
    if (qapunte!="")
       {
        if (basedatos::instancia()->pase_en_facturas(qapunte,&serie,&numero))
           {
            QString fichero=latex_doc(serie,numero);
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
       }
    return;
  }


  fichdoc=expanderutadocfich(fichdoc);

   if (!ejecuta(aplicacionabrirfich(extensionfich(fichdoc)),fichdoc))
         QMessageBox::warning( this, tr("TABLA DE ASIENTOS"),
                             tr("No se puede abrir ")+fichdoc+tr(" con ")+
                             aplicacionabrirfich(extensionfich(fichdoc)));

}



void consmayor::latex()
{
    int valor=0;
    if (ui.intervalogroupBox->isChecked())
       {
        latexintervalo();
        valor=editalatex(tr("mayor_int"));
       }
      else
       {
        cuadimprimayor *cuadimpri=new cuadimprimayor(comadecimal,decimales);
        cuadimpri->generalatexmayor( ui.subcuentalineEdit->text(),ui.subcuentalineEdit->text(),
                                  ui.inicialdateEdit->date(), ui.finaldateEdit->date() );
        delete cuadimpri;
        valor=editalatex("mayor");
       }
    if (valor==1)
        QMessageBox::warning( this, tr("Consulta de mayor"),tr("PROBLEMAS al llamar al editor Latex"));

}


void consmayor::consultaasiento()
{

    QString asiento;

    if (ui.mayortable->currentItem()==0) return;
    int dia=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().left(2).toInt();
    int mes=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().mid(3,2).toInt();
    int anyo=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().right(4).toInt();
    QDate fecha(anyo,mes,dia);
    asiento=ui.mayortable->item(ui.mayortable->currentRow(),2)->text();

    QString ejercicio=ejerciciodelafecha(fecha);
    QString qdiario;
    qdiario=diariodeasiento(asiento,ejercicio);

    if (qdiario==diario_apertura() || qdiario==diario_cierre() || qdiario==diario_regularizacion())
       {
         QMessageBox::warning( this, tr("Consultas de Mayor"),
                               tr("ERROR: No se pueden consultar asientos del diario de apertura, "
                                  "regularización o cierre\n"));
         return;
       }

    if (!existeasiento(asiento,ejercicio))
       {
         QMessageBox::warning( this, tr("Consultas de Mayor"),
                               tr("ERROR: El asiento seleccionado ya no existe"));
         return;
       }

    consultarasiento(asiento,usuario,ejercicio);

    // refrescar();

}


void consmayor::datoscuenta()
{
    QString cuenta=ui.subcuentalineEdit->text();
    if (cuenta.isEmpty()) return;
    if (!existesubcuenta(cuenta)) return;
    datos_accesorios *d = new datos_accesorios();
    d->cargacodigo( cuenta );
    d->exec();
    delete(d);
}


void consmayor::vencimientos()
{

    QString asiento;

    if (ui.mayortable->currentItem()==0) return;

    int dia=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().left(2).toInt();
    int mes=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().mid(3,2).toInt();
    int anyo=ui.mayortable->item(ui.mayortable->currentRow(),1)->text().right(4).toInt();
    QDate fecha(anyo,mes,dia);
    asiento=ui.mayortable->item(ui.mayortable->currentRow(),2)->text();

    QString cadnumpase=ui.mayortable->item(ui.mayortable->currentRow(),10)->text();


    if (espasevtoprocesado(cadnumpase))
       {
         QMessageBox::warning( this, tr("CONSULTA VENCIMIENTOS EN PASE DIARIO"),
         tr("El pase seleccionado es un cobro o pago ya efectuado"));
         return;
       }
    QString caddebe;
    caddebe=convapunto(ui.mayortable->item(ui.mayortable->currentRow(),4)->text());
    QString cadhaber;
    cadhaber=convapunto(ui.mayortable->item(ui.mayortable->currentRow(),5)->text());
    QString subcuenta=ui.mayortable->item(ui.mayortable->currentRow(),0)->text();
    consultavencipase *losvenci=new consultavencipase(cadnumpase,
                              asiento,
                              caddebe,
                              cadhaber,
                              subcuenta,
                              fecha);
    losvenci->exec();
    delete(losvenci);

}


void consmayor::nuevo_asiento()
{
    introfecha i(QDate::currentDate());
    i.esconde_herramientas();
    i.adjustSize();
    int resultado=i.exec();
    QDate fecha=i.fecha();
    QString diario=i.diario();

    /*introfecha *i = new introfecha(QDate::currentDate());
    int resultado=i->exec();
    QDate fecha=i->fecha();
    QString diario=i->diario();
    delete(i); */
    if (!resultado==QDialog::Accepted) return;
    tabla_asientos *t = new tabla_asientos(comadecimal,decimales,usuario);
    t->pasafechaasiento(fecha);
    t->pasadiario(diario);
    resultado=t->exec();
    delete(t);

    if (resultado!=QDialog::Accepted) return;

    refrescar();

}


void consmayor::procesa_vencimiento()
{
    if (ui.mayortable->currentItem()==0) return;

    QString apunte=ui.mayortable->item(ui.mayortable->currentRow(),10)->text();

    int numvenci = basedatos::instancia()->primer_venci_pte_de_pase(apunte);
    if (numvenci>=0)
      {
        QString cadnum; cadnum.setNum(numvenci);
        procesavencimiento *procvenci=new procesavencimiento(usuario);
        procvenci->cargadatos(cadnum);
        int resultado=procvenci->exec();
        if (resultado!=QDialog::Accepted)
           {
             QMessageBox::information( this, tr("Vencimientos"),tr("El procesado del vencimiento se ha cancelado"));
           }
           else refrescar();
        delete procvenci;
      }

}


void consmayor::acumuladoscta()
{
    acumuladosmes *a = new acumuladosmes();
    a->pasacodigocuenta(ui.subcuentalineEdit->text(),ui.inicialdateEdit->date());
    a->exec();
    delete(a);
}


void consmayor::asigna_ficherodocumento()
{
    if (ui.mayortable->currentItem()==0) return;


    QString elasiento=ui.mayortable->item(ui.mayortable->currentRow(),2)->text();

    QString cadnumpase=ui.mayortable->item(ui.mayortable->currentRow(),10)->text();

    QString documento=ui.mayortable->item(ui.mayortable->currentRow(),7)->text();

    QDate fechapase=basedatos::instancia()->fecha_pase_diario(cadnumpase);

    QString ejercicio=ejerciciodelafecha(fechapase);

    if (ejerciciocerrado(ejercicio) || ejerciciocerrando(ejercicio))
     {
        QMessageBox::warning( this, tr("ASIGNAR FICHERO A DOCUMENTO"),
                              tr("ERROR: ejercicio cerrado"));
        return;
     }

    // QMessageBox::warning( this, tr("ASIGNAR FICHERO A DOCUMENTO"),
    //                      punterodiario->documentoactual());

    asigna_fichdoc *a = new asigna_fichdoc(elasiento,documento,
                                           ejercicio);
    a->pasafichdoc(basedatos::instancia()->copia_docdiario (elasiento,ejercicio));

    a->exec();
    delete(a);

}
