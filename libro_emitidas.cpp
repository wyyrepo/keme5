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

#include "libro_emitidas.h"
#include <QSqlQuery>
#include "funciones.h"
#include "basedatos.h"
#include "tabla_iva_rep.h"
#include <QMessageBox>
#include "buscasubcuenta.h"


libro_emitidas::libro_emitidas(QString qusuario) : QDialog() {
    ui.setupUi(this);

usuario=qusuario;

  QSqlQuery query = basedatos::instancia()->selectCodigoejerciciosordercodigo();
  QStringList ej1;

  if ( query.isActive() ) {
          while ( query.next() )
                ej1 << query.value(0).toString();
	  }
  ui.ejerciciocomboBox->addItems(ej1);

  comadecimal=haycomadecimal(); decimales=haydecimales();

  connect(ui.consultarpushButton,SIGNAL( clicked()),this,SLOT(consultatex()));
  connect(ui.imprimirpushButton,SIGNAL( clicked()),this,SLOT(imprime()));
  connect(ui.copiarpushButton,SIGNAL( clicked()),this,SLOT(copiar()));

  connect(ui.bicheckBox ,SIGNAL(stateChanged(int)),SLOT(bicheckcambiado()));
  connect(ui.latexpushButton,SIGNAL(clicked()),SLOT(latex()));

  connect(ui.tablapushButton,SIGNAL(clicked()),SLOT(tabla()));

  connect(ui.codigopushButton,SIGNAL(clicked(bool)),SLOT(boton_cuenta_pulsado()));
  connect(ui.cuentalineEdit,SIGNAL(editingFinished()),this,SLOT(cuenta_finedicion()));
  connect(ui.cuentalineEdit,SIGNAL(textChanged(QString)),this,SLOT(cuenta_cambiada()));

  ui.inicialdateEdit->setDate(QDate::currentDate());
  ui.finaldateEdit->setDate(QDate::currentDate());


#ifdef NOEDITTEX
 ui.latexpushButton->hide();
#endif

}

bool libro_emitidas::generalatexrepercutido()
{
   if (!ui.interiorescheckBox->isChecked() && !ui.aibcheckBox->isChecked() && 
       !ui.eibcheckBox->isChecked() && !ui.autofacturascheckBox->isChecked() &&
       !ui.rectificativascheckBox->isChecked()
       && !ui.autofacturasnouecheckBox->isChecked() && !ui.prestserviciosuecheckBox->isChecked()
       && !ui.exportacionescheckBox->isChecked() && !ui.isp_op_interiorescheckBox->isChecked()
       && !ui.exentas_no_deduccioncheckBox->isChecked()) return false;

   QString qfichero=dirtrabajo();
   qfichero.append(QDir::separator());
   qfichero+=nombrefichero();
   qfichero+=".tex";
   // QMessageBox::warning( this, tr("Estados Contables"),qfichero);
   QFile fichero(adapta(qfichero));
   if (! fichero.open( QIODevice::WriteOnly ) ) return false;

    QTextStream stream( &fichero );
    stream.setCodec("UTF-8");
    stream << cabeceralatex();
    stream << margen_extra_latex();

    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {";
    stream << filtracad(nombreempresa()) << "}}";
    stream << "\\end{center}";

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    if (!cuenta.isEmpty())
     {
      stream << "\\begin{center}" << "\n";
      stream << "{\\Large \\textbf {";
      stream << filtracad(cuenta) + " " + descripcioncuenta(cuenta) << "}}";
      stream << "\\end{center}";
     }

      stream << "\\begin{center}" << "\n";
      stream << "\\begin{longtable}{|r|p{2cm}|c|p{3.5cm}|r|r|r|r|r|r|r|}" << "\n";
      stream << "\\hline" << "\n";

   stream << "\\multicolumn{11}{|c|} {\\textbf{";

   QString cadena=filtracad(ui.cabeceralineEdit->text());

    // --------------------------------------------------------------------------------------
    stream << cadena;
    stream <<  "}} \\\\";
    stream << "\\hline" << "\n";
    // ---------------------------------------------------------------------------------------
    stream << "{\\tiny{" << tr("Orden") << "}} & ";
    stream << "{\\tiny{" << tr("Factura") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha Fra.") << "}} & ";
    // stream << "{\\tiny{" << tr("Contab.") << "}} & ";
    stream << "{\\tiny{" << tr("Cuenta, Cliente/deudor") << "}} & ";
    stream << "{\\tiny{" << tr("Asto.") << "}} & ";
    stream << "{\\tiny{" << tr("Base Imp.") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo") << "}} & ";
    stream << "{\\tiny{" << tr("Cuota") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo.RE") << "}} & ";
    stream << "{\\tiny{" << tr("R.E.") << "}} & ";
    stream << "{\\tiny{" << tr("Total") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endfirsthead";
    // --------------------------------------------------------------------------------------------------
    stream << "\\hline" << "\n";
    // stream << tr(" \\\\") << "\n";
    stream << "{\\tiny{" << tr("Orden") << "}} & ";
    stream << "{\\tiny{" << tr("Factura") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha Fra.") << "}} & ";
    // stream << "{\\tiny{" << tr("Contab.") << "}} & ";
    stream << "{\\tiny{" << tr("Cuenta, Cliente/deudor") << "}} & ";
    stream << "{\\tiny{" << tr("Asto.") << "}} & ";
    stream << "{\\tiny{" << tr("Base Imp.") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo") << "}} & ";
    stream << "{\\tiny{" << tr("Cuota") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo.RE") << "}} & ";
    stream << "{\\tiny{" << tr("R.E.") << "}} & ";
    stream << "{\\tiny{" << tr("Total") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endhead" << "\n";
    // -----------------------------------------------------------------------------------------------

    QString ejercicio=ui.ejerciciocomboBox->currentText();
    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    int registros = basedatos::instancia()->num_registros_emitidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.eibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.prestserviciosuecheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.exportacionescheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         !ui.exentas_no_deduccioncheckBox->isChecked(), cuenta);

    // ÚLTIMOS PARÁMETROS
    // bool filtrarnosujetas, bool solonosujetas, bool sinnosujetas
    bool filtrarfechafactura=false;
    bool filtrardoc=false;
    if (ui.ordengroupBox->isChecked())
      {
       filtrarfechafactura=ui.ordenfechafacradioButton->isChecked();
       filtrardoc=ui.ordendocradioButton->isChecked();
      }
    QSqlQuery query =
      basedatos::instancia()->registros_emitidas(ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.eibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.prestserviciosuecheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.exportacionescheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         !ui.exentas_no_deduccioncheckBox->isChecked(), cuenta,
         filtrarfechafactura, filtrardoc);

    ui.progressBar->setMaximum(registros);
    int numorden=0;
    int numprog=1;
    QString cadnum,guardadoc,guardaprov;
    double base=0;
    double cuota=0;
    double re=0;
    if ( query.isActive() ) {
          while ( query.next() )
               {
	        ui.progressBar->setValue(numprog);
                if (guardadoc!=query.value(0).toString() || query.value(0).toString().length()==0 
                    || numprog==1 || guardaprov!=query.value(2).toString()) numorden++;
                guardadoc=query.value(0).toString();
                guardaprov=query.value(2).toString();
	        stream << "{\\tiny " << cadnum.setNum(numorden) << "} & {\\tiny "; 
	        stream << filtracad(query.value(0).toString()) << "} & {\\tiny "; // fra.
	        stream << query.value(11).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha fra.
	        // stream << query.value(1).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha cont.
	        stream <<  query.value(2).toString() << " "; // cuenta
                stream << (query.value(13).toString().isEmpty() ?
                           filtracad(query.value(3).toString()) :
                           filtracad(query.value(13).toString())) << " "; // proveedor
                stream << (query.value(14).toString().isEmpty() ?
                           filtracad(cifcta(query.value(2).toString())) :
                           filtracad(query.value(14).toString()))
                           << "} & {\\tiny "; // cif
		
                stream <<  query.value(4).toString() << "} & {\\tiny "; // asiento

                stream <<  formatea_redondeado_sep(query.value(5).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // base iva
                stream <<  formatea_redondeado_sep(query.value(6).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // tipo
                double qcuota=CutAndRoundNumberToNDecimals(query.value(5).toDouble()
                       *query.value(6).toDouble()/100,2);
                stream <<  formatea_redondeado_sep(qcuota,comadecimal,decimales)
                       << "} & {\\tiny "; // cuota
                stream <<  formatea_redondeado_sep(query.value(8).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // tipo re
                double qre= CutAndRoundNumberToNDecimals(query.value(5).toDouble()*
                         query.value(8).toDouble()/100,2);
                stream <<  formatea_redondeado_sep(qre,comadecimal,decimales)
                       << "} & {\\tiny "; // re
                stream <<  formatea_redondeado_sep(query.value(10).toDouble(),comadecimal,decimales)
                       << "} \\\\ \n  " << "\\hline\n"; // total
                if (query.value(16).toBool())
                   {
                    // criterio de caja
                    QSqlQuery q=basedatos::instancia()->datos_venci_apunte_iva_caja_liquidados(
                            query.value(15).toString());
                    // v.fecha_vencimiento, v.liq_iva_fecha, v.forzar_liq_iva, v.importe,
                    // v.num, v.medio_realizacion, v.cuenta_bancaria
                    bool vacio=true;
                    if (q.isActive())
                        while (q.next())
                         {
                          vacio=false;
                          stream << "\\multicolumn{2}{|r|} {\\tiny ";
                          stream << tr("Importe pago");
                          stream << "} & {\\tiny ";
                          stream << formatea_redondeado_sep(q.value(3).toDouble(),comadecimal,decimales);
                          stream << "} & \\multicolumn{2}{|l|} {\\tiny ";
                          stream << q.value(5).toString();
                          stream << "} & \\multicolumn{2}{|r|} {\\tiny ";
                          stream << tr("FECHA: ");
                          // stream << "} & {\\tiny ";
                          stream << q.value(1).toDate().toString("dd-MM-yyyy");
                          stream << "} & {\\tiny ";
                          stream << tr("Cuenta:");
                          stream << "} & \\multicolumn{3}{|l|} {\\tiny ";
                          stream << q.value(6).toString();
                          stream << "} \\\\ \n  "
                                  << "\\hline\n";
                         }
                    if (vacio)
                       {
                        stream << "\\multicolumn{2}{|r|} {\\tiny ";
                        stream << tr("Importe pago");
                        stream << "} & {\\tiny ";
                        stream << "} & {\\tiny ";
                        stream << tr("FECHA:");
                        stream << "} & {\\tiny ";
                        stream << "} & \\multicolumn{2}{|r|} {\\tiny ";
                        stream << "} & {\\tiny ";
                        stream << tr("Cuenta:");
                        stream << "} & \\multicolumn{3}{|l|} {\\tiny ";
                        stream << "} \\\\ \n  "
                             << "\\hline\n";
                       }
                   }
                numprog++;
	        base+=query.value(5).toDouble();
                cuota+=qcuota;
                re+=qre;
	      }
      }

    stream << "\\multicolumn{5}{|r|} {\\textbf{";
   // --------------------------------------------------------------------------------------
    stream << tr("SUMAS TOTALES:")+"}} ";
    stream << " & {\\tiny " << formatea_redondeado_sep(base,comadecimal,decimales) << "} & & {\\tiny ";
    stream << formatea_redondeado_sep(cuota,comadecimal,decimales) << "} & & {\\tiny ";
    stream << formatea_redondeado_sep(re,comadecimal,decimales) << "} &  {\\tiny ";
    stream << formatea_redondeado_sep(base+cuota+re,comadecimal,decimales);
    stream <<  "} \\\\ \n";
    stream << "\\hline" << "\n";


   stream << "\\end{longtable}" << "\n";
   stream << "\\end{center}" << "\n";

    stream << "% FIN_CUERPO\n";
    stream << "\\end{document}" << "\n";

    fichero.close();

    ui.progressBar->reset();

    return true;
}





void libro_emitidas::consultatex()
{
   if (!ui.horizontalradioButton->isChecked())
     {
      if  (!generalatexrepercutido())
         return;
     }

   if (ui.horizontalradioButton->isChecked())
     {
      if (ui.bicheckBox->isChecked())
       {
          if  (!generalatexrepercutido_horizontal_bi())
             return;
       }
       else
          {
           if  (!generalatexrepercutido_horizontal())
            return;
          }
     }

   int valor=consultalatex2(nombrefichero());
   if (valor==1)
       QMessageBox::warning( this, tr("Libros de FACTURAS"),tr("PROBLEMAS al llamar a Latex"));
   if (valor==2)
       QMessageBox::warning( this, tr("Libros de FACTURAS"),
                             tr("PROBLEMAS al llamar a ")+visordvi());


}


void libro_emitidas::imprime()
{
   if (!ui.horizontalradioButton->isChecked())
     {
      if  (!generalatexrepercutido())
         return;
     }

   if (ui.horizontalradioButton->isChecked())
     {
      if  (!generalatexrepercutido_horizontal())
         return;
     }

   int valor=imprimelatex2(nombrefichero());
   if (valor==1)
       QMessageBox::warning( this, tr("Libros de IVA"),tr("PROBLEMAS al llamar a Latex"));
   if (valor==2)
       QMessageBox::warning( this, tr("Libros de IVA"),
                                tr("PROBLEMAS al llamar a 'dvips'"));
   if (valor==3)
       QMessageBox::warning( this, tr("Libros de IVA"),
                             tr("PROBLEMAS al llamar a ")+programa_imprimir());

}


QString libro_emitidas::nombrefichero()
{
  QString cadena;
  cadena=tr("facturas_emitidas");
  return cadena;
}


void libro_emitidas::copiar()
{
   if (!ui.horizontalradioButton->isChecked())
     {
       copiavertical();
       return;
     }

   if (ui.horizontalradioButton->isChecked())
     {
       if (ui.bicheckBox->isChecked()) copiahorizontal_bi();
         else copiahorizontal();
      return;
     }

}


bool libro_emitidas::generalatexrepercutido_horizontal()
{
   if (!ui.interiorescheckBox->isChecked() && !ui.aibcheckBox->isChecked() && 
       !ui.eibcheckBox->isChecked() && !ui.autofacturascheckBox->isChecked() &&
       !ui.rectificativascheckBox->isChecked()
       && !ui.autofacturasnouecheckBox->isChecked() && !ui.prestserviciosuecheckBox->isChecked()
       && !ui.exportacionescheckBox->isChecked() && !ui.isp_op_interiorescheckBox->isChecked()
       && !ui.exentas_no_deduccioncheckBox->isChecked())
        return false;

   QString qfichero=dirtrabajo();
   qfichero.append(QDir::separator());
   qfichero+=nombrefichero();
   qfichero+=".tex";
   // QMessageBox::warning( this, tr("Estados Contables"),qfichero);
   QFile fichero(adapta(qfichero));
   if (! fichero.open( QIODevice::WriteOnly ) ) return false;

    QTextStream stream( &fichero );
    stream.setCodec("UTF-8");
    stream << cabeceralatex();
    stream << margen_extra_latex();

    stream << "\\begin{landscape}" << "\n";

    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {";
    stream << filtracad(nombreempresa()) << "}}";
    stream << "\\end{center}";

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    if (!cuenta.isEmpty())
     {
      stream << "\\begin{center}" << "\n";
      stream << "{\\Large \\textbf {";
      stream << filtracad(cuenta) + " " + descripcioncuenta(cuenta) << "}}";
      stream << "\\end{center}";
     }

    stream << "\\begin{center}" << "\n";
    stream << "\\begin{longtable}{|r|p{2.5cm}|c|c|p{6cm}|r|r|r|r|r|r|r|}" << "\n";
    stream << "\\hline" << "\n";

   stream << "\\multicolumn{12}{|c|} {\\textbf{";

   QString cadena=ui.cabeceralineEdit->text();

    // --------------------------------------------------------------------------------------
    stream << cadena;
    stream <<  "}} \\\\";
    stream << "\\hline" << "\n";
    // ---------------------------------------------------------------------------------------
    stream << "{\\tiny{" << tr("Orden") << "}} & ";
    stream << "{\\tiny{" << tr("Factura") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha Fra.") << "}} & ";
    stream << "{\\tiny{" << tr("Contab.") << "}} & ";
    stream << "{\\tiny{" << tr("Cuenta, Cliente/deudor") << "}} & ";
    stream << "{\\tiny{" << tr("Asto.") << "}} & ";
    stream << "{\\tiny{" << tr("Base Imp.") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo") << "}} & ";
    stream << "{\\tiny{" << tr("Cuota") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo.RE") << "}} & ";
    stream << "{\\tiny{" << tr("R.E.") << "}} & ";
    stream << "{\\tiny{" << tr("Total") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endfirsthead";
    // --------------------------------------------------------------------------------------------------
    stream << "\\hline" << "\n";
    // stream << tr(" \\\\") << "\n";
    stream << "{\\tiny{" << tr("Orden") << "}} & ";
    stream << "{\\tiny{" << tr("Factura") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha Fra.") << "}} & ";
    stream << "{\\tiny{" << tr("Contab.") << "}} & ";
    stream << "{\\tiny{" << tr("Cuenta, Cliente/deudor") << "}} & ";
    stream << "{\\tiny{" << tr("Asto.") << "}} & ";
    stream << "{\\tiny{" << tr("Base Imp.") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo") << "}} & ";
    stream << "{\\tiny{" << tr("Cuota") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo.RE") << "}} & ";
    stream << "{\\tiny{" << tr("R.E.") << "}} & ";
    stream << "{\\tiny{" << tr("Total") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endhead" << "\n";
    // -----------------------------------------------------------------------------------------------

    QString ejercicio=ui.ejerciciocomboBox->currentText();

    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    int registros = basedatos::instancia()->num_registros_emitidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.eibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.prestserviciosuecheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.exportacionescheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         !ui.exentas_no_deduccioncheckBox->isChecked(),cuenta );

    bool filtrarfechafactura=false;
    bool filtrardoc=false;
    if (ui.ordengroupBox->isChecked())
      {
       filtrarfechafactura=ui.ordenfechafacradioButton->isChecked();
       filtrardoc=ui.ordendocradioButton->isChecked();
      }


    QSqlQuery query =
      basedatos::instancia()->registros_emitidas(ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.eibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.prestserviciosuecheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.exportacionescheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         !ui.exentas_no_deduccioncheckBox->isChecked(),cuenta , filtrarfechafactura, filtrardoc);

    ui.progressBar->setMaximum(registros);
    int numorden=0;
    int numprog=1;
    QString cadnum,guardadoc,guardaprov;
    double base=0;
    double cuota=0;
    double re=0;
    if ( query.isActive() ) {
          while ( query.next() )
               {
	        ui.progressBar->setValue(numprog);
                if (guardadoc!=query.value(0).toString() || query.value(0).toString().length()==0 
                    || numprog==1 || guardaprov!=query.value(2).toString()) numorden++;
                guardadoc=query.value(0).toString();
                guardaprov=query.value(2).toString();
	        stream << "{\\tiny " << cadnum.setNum(numorden) << "} & {\\tiny "; 
	        stream << filtracad(query.value(0).toString()) << "} & {\\tiny "; // fra.
	        stream << query.value(11).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha fra.
	        stream << query.value(1).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha cont.
	        stream <<  query.value(2).toString() << " "; // cuenta
                stream << (query.value(13).toString().isEmpty() ?
                           filtracad(query.value(3).toString()) :
                           filtracad(query.value(13).toString())) << " "; // proveedor
                stream << (query.value(14).toString().isEmpty() ?
                           filtracad(cifcta(query.value(2).toString())) :
                           filtracad(query.value(14).toString()))
                           << "} & {\\tiny "; // cif

                stream <<  query.value(4).toString() << "} & {\\tiny "; // asiento

                stream <<  formatea_redondeado_sep(query.value(5).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // base iva
                stream <<  formatea_redondeado_sep(query.value(6).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // tipo
                double qcuota=CutAndRoundNumberToNDecimals(query.value(5).toDouble()*
                                                 query.value(6).toDouble()/100,2);
                stream <<  formatea_redondeado_sep(qcuota,comadecimal,decimales)
                       << "} & {\\tiny "; // cuota
                stream <<  formatea_redondeado_sep(query.value(8).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // tipo re
                double qre=CutAndRoundNumberToNDecimals (query.value(5).toDouble()*
                                              query.value(8).toDouble()/100,2);
                stream <<  formatea_redondeado_sep(qre,comadecimal,decimales)
                       << "} & {\\tiny "; // re
                stream <<  formatea_redondeado_sep(query.value(10).toDouble(),comadecimal,decimales)
                       << "} \\\\ \n  " << "\\hline\n"; // total
                if (query.value(16).toBool())
                   {
                    // criterio de caja
                    QSqlQuery q=basedatos::instancia()->datos_venci_apunte_iva_caja_liquidados(
                            query.value(15).toString());
                    // v.fecha_vencimiento, v.liq_iva_fecha, v.forzar_liq_iva, v.importe,
                    // v.num, v.medio_realizacion, v.cuenta_bancaria
                    bool vacio=true;
                    if (q.isActive())
                        while (q.next())
                         {
                          vacio=false;
                          stream << "\\multicolumn{2}{|r|} {\\tiny ";
                          stream << tr("Importe pago");
                          stream << "} & {\\tiny ";
                          stream << formatea_redondeado_sep(q.value(3).toDouble(),comadecimal,decimales);
                          stream << "} & \\multicolumn{2}{|l|} {\\tiny ";
                          stream << q.value(5).toString();
                          stream << "} & \\multicolumn{2}{|r|} {\\tiny ";
                          stream << tr("FECHA: ");
                          // stream << "} & {\\tiny ";
                          stream << q.value(1).toDate().toString("dd-MM-yyyy");
                          stream << "} &  & {\\tiny ";
                          stream << tr("Cuenta:");
                          stream << "} & \\multicolumn{3}{|l|} {\\tiny ";
                          stream << q.value(6).toString();
                          stream << "} \\\\ \n  "
                                  << "\\hline\n";
                         }
                    if (vacio)
                       {
                        stream << "\\multicolumn{2}{|r|} {\\tiny ";
                        stream << tr("Importe pago");
                        stream << "} & {\\tiny ";
                        stream << "} & {\\tiny ";
                        stream << tr("FECHA:");
                        stream << "} & {\\tiny ";
                        stream << "} & \\multicolumn{2}{|r|} {\\tiny ";
                        stream << "} & & {\\tiny ";
                        stream << tr("Cuenta:");
                        stream << "} & \\multicolumn{3}{|l|} {\\tiny ";
                        stream << "} \\\\ \n  "
                             << "\\hline\n";
                       }
                   }
                numprog++;
	        base+=query.value(5).toDouble();
                cuota+=qcuota;
                re+=qre;
	      }
      }

    stream << "\\multicolumn{6}{|r|} {\\textbf{";
   // --------------------------------------------------------------------------------------
    stream << tr("SUMAS TOTALES:")+"}} ";
    stream << " & {\\tiny " << formatea_redondeado_sep(base,comadecimal,decimales) << "} & & {\\tiny ";
    stream << formatea_redondeado_sep(cuota,comadecimal,decimales) << "} & & {\\tiny ";
    stream << formatea_redondeado_sep(re,comadecimal,decimales) << "} &  {\\tiny ";
    stream << formatea_redondeado_sep(base+cuota+re,comadecimal,decimales);
    stream <<  "} \\\\ \n";
    stream << "\\hline" << "\n";


   stream << "\\end{longtable}" << "\n";
   stream << "\\end{center}" << "\n";

    stream << "\\end{landscape}\n";

    stream << "% FIN_CUERPO\n";
    stream << "\\end{document}" << "\n";

    fichero.close();

    ui.progressBar->reset();

    return true;

}


bool libro_emitidas::generalatexrepercutido_horizontal_bi()
{
   if (!ui.interiorescheckBox->isChecked() && !ui.aibcheckBox->isChecked() &&
       !ui.eibcheckBox->isChecked() && !ui.autofacturascheckBox->isChecked() &&
       !ui.rectificativascheckBox->isChecked()
       && !ui.autofacturasnouecheckBox->isChecked() && !ui.prestserviciosuecheckBox->isChecked()
       && !ui.exportacionescheckBox->isChecked() && !ui.isp_op_interiorescheckBox->isChecked()
       && !ui.exentas_no_deduccioncheckBox->isChecked())
        return false;

   QString qfichero=dirtrabajo();
   qfichero.append(QDir::separator());
   qfichero+=nombrefichero();
   qfichero+=".tex";
   // QMessageBox::warning( this, tr("Estados Contables"),qfichero);
   QFile fichero(adapta(qfichero));
   if (! fichero.open( QIODevice::WriteOnly ) ) return false;

    QTextStream stream( &fichero );
    stream.setCodec("UTF-8");
    stream << cabeceralatex();
    stream << margen_extra_latex();

    stream << "\\begin{landscape}" << "\n";

    stream << "\\begin{center}" << "\n";
    stream << "{\\Large \\textbf {";
    stream << filtracad(nombreempresa()) << "}}";
    stream << "\\end{center}";

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    if (!cuenta.isEmpty())
     {
      stream << "\\begin{center}" << "\n";
      stream << "{\\Large \\textbf {";
      stream << filtracad(cuenta) + " " + descripcioncuenta(cuenta) << "}}";
      stream << "\\end{center}";
     }

    stream << "\\begin{center}" << "\n";
    stream << "\\begin{longtable}{|r|p{2.5cm}|c|c|p{4cm}|p{4cm}|r|r|r|r|r|r|r|}" << "\n";
    stream << "\\hline" << "\n";

   stream << "\\multicolumn{13}{|c|} {\\textbf{";

   QString cadena=filtracad(ui.cabeceralineEdit->text());

    // --------------------------------------------------------------------------------------
    stream << cadena;
    stream <<  "}} \\\\";
    stream << "\\hline" << "\n";
    // ---------------------------------------------------------------------------------------
    stream << "{\\tiny{" << tr("Orden") << "}} & ";
    stream << "{\\tiny{" << tr("Factura") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha Fra.") << "}} & ";
    stream << "{\\tiny{" << tr("Contab.") << "}} & ";
    stream << "{\\tiny{" << tr("Cuenta, Cliente/deudor") << "}} & ";
    stream << "{\\tiny{" << tr("Concepto") << "}} & ";
    stream << "{\\tiny{" << tr("Asto.") << "}} & ";
    stream << "{\\tiny{" << tr("Base Imp.") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo") << "}} & ";
    stream << "{\\tiny{" << tr("Cuota") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo.RE") << "}} & ";
    stream << "{\\tiny{" << tr("R.E.") << "}} & ";
    stream << "{\\tiny{" << tr("Total") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endfirsthead";
    // --------------------------------------------------------------------------------------------------
    stream << "\\hline" << "\n";
    // stream << tr(" \\\\") << "\n";
    stream << "{\\tiny{" << tr("Orden") << "}} & ";
    stream << "{\\tiny{" << tr("Factura") << "}} & ";
    stream << "{\\tiny{" << tr("Fecha Fra.") << "}} & ";
    stream << "{\\tiny{" << tr("Contab.") << "}} & ";
    stream << "{\\tiny{" << tr("Cuenta, Cliente/deudor") << "}} & ";
    stream << "{\\tiny{" << tr("Concepto") << "}} & ";
    stream << "{\\tiny{" << tr("Asto.") << "}} & ";
    stream << "{\\tiny{" << tr("Base Imp.") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo") << "}} & ";
    stream << "{\\tiny{" << tr("Cuota") << "}} & ";
    stream << "{\\tiny{" << tr("Tipo.RE") << "}} & ";
    stream << "{\\tiny{" << tr("R.E.") << "}} & ";
    stream << "{\\tiny{" << tr("Total") << "}}";
    stream << " \\\\" << "\n";
    stream << "\\hline" << "\n";
    stream << "\\endhead" << "\n";
    // -----------------------------------------------------------------------------------------------

    QString ejercicio=ui.ejerciciocomboBox->currentText();

    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    int registros = basedatos::instancia()->num_registros_emitidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.eibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.prestserviciosuecheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.exportacionescheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         !ui.exentas_no_deduccioncheckBox->isChecked(),cuenta);

    bool filtrarfechafactura=false;
    bool filtrardoc=false;
    if (ui.ordengroupBox->isChecked())
      {
       filtrarfechafactura=ui.ordenfechafacradioButton->isChecked();
       filtrardoc=ui.ordendocradioButton->isChecked();
      }

    QSqlQuery query =
      basedatos::instancia()->registros_emitidas(ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.eibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.prestserviciosuecheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.exportacionescheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         !ui.exentas_no_deduccioncheckBox->isChecked(), cuenta , filtrarfechafactura,filtrardoc);

    ui.progressBar->setMaximum(registros);
    int numorden=0;
    int numprog=1;
    QString cadnum,guardadoc,guardaprov;
    double base=0;
    double cuota=0;
    double re=0;
    if ( query.isActive() ) {
          while ( query.next() )
               {
                ui.progressBar->setValue(numprog);
                if (guardadoc!=query.value(0).toString() || query.value(0).toString().length()==0
                    || numprog==1 || guardaprov!=query.value(2).toString()) numorden++;
                guardadoc=query.value(0).toString();
                guardaprov=query.value(2).toString();
                stream << "{\\tiny " << cadnum.setNum(numorden) << "} & {\\tiny ";
                stream << filtracad(query.value(0).toString()) << "} & {\\tiny "; // fra.
                stream << query.value(11).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha fra.
                stream << query.value(1).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha cont.
                stream <<  query.value(2).toString() << " "; // cuenta
                stream <<  ( query.value(13).toString().isEmpty() ?
                             filtracad(query.value(3).toString()) :
                             filtracad(query.value(13).toString()) )
                             << " "; // proveedor
                stream << (query.value(14).toString().isEmpty() ?
                           filtracad(cifcta(query.value(2).toString())) :
                           filtracad(query.value(14).toString()))
                           << "} & {\\tiny "; // cif
                stream <<  filtracad(descripcioncuenta(query.value(12).toString())) << "} & {\\tiny "; // concepto

                stream <<  query.value(4).toString() << "} & {\\tiny "; // asiento

                stream <<  formatea_redondeado_sep(query.value(5).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // base iva
                stream <<  formatea_redondeado_sep(query.value(6).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // tipo
                QString qcuota=formateanumero(query.value(5).toDouble()*
                                                 query.value(6).toDouble()/100,comadecimal,decimales);
                stream <<  formatea_redondeado_sep(query.value(5).toDouble()*
                                             query.value(6).toDouble()/100,comadecimal,decimales)
                       << "} & {\\tiny "; // cuota
                stream <<  formatea_redondeado_sep(query.value(8).toDouble(),comadecimal,decimales)
                       << "} & {\\tiny "; // tipo re
                QString qre=formateanumero(query.value(5).toDouble()*
                                              query.value(8).toDouble()/100,comadecimal,decimales);
                stream <<  formatea_redondeado_sep(query.value(5).toDouble()*
                                              query.value(8).toDouble()/100,comadecimal,decimales)
                       << "} & {\\tiny "; // re
                stream <<  formatea_redondeado_sep(query.value(10).toDouble(),comadecimal,decimales)
                       << "} \\\\ \n  " << "\\hline\n"; // total
                numprog++;
                base+=query.value(5).toDouble();
                cuota+=convapunto(qcuota).toDouble();
                re+=convapunto(qre).toDouble();
              }
      }

    stream << "\\multicolumn{7}{|r|} {\\textbf{";
   // --------------------------------------------------------------------------------------
    stream << tr("SUMAS TOTALES:")+"}} ";
    stream << " & {\\tiny " << formatea_redondeado_sep(base,comadecimal,decimales) << "} & & {\\tiny ";
    stream << formatea_redondeado_sep(cuota,comadecimal,decimales) << "} & & {\\tiny ";
    stream << formatea_redondeado_sep(re,comadecimal,decimales) << "} &  {\\tiny ";
    stream << formatea_redondeado_sep(base+cuota+re,comadecimal,decimales);
    stream <<  "} \\\\ \n";
    stream << "\\hline" << "\n";


   stream << "\\end{longtable}" << "\n";
   stream << "\\end{center}" << "\n";

    stream << "\\end{landscape}\n";

    stream << "% FIN_CUERPO\n";
    stream << "\\end{document}" << "\n";

    fichero.close();

    ui.progressBar->reset();

    return true;

}


void libro_emitidas::copiavertical()
{
   QClipboard *cb = QApplication::clipboard();
   QString global;


   global=filtracad(nombreempresa())+"\n\n";

   QString cadena=ui.cabeceralineEdit->text();

    // --------------------------------------------------------------------------------------
   global+=cadena+"\n\n";
    // ---------------------------------------------------------------------------------------
     global+=tr("Orden")+"\t";
     global+=tr("Factura")+"\t";
     global+=tr("Fecha Fra.")+"\t";
    // stream << "{\\tiny{" << tr("Contab.") << "}} & ";
     global+=tr("Cuenta, Cliente/deudor")+"\t";
     global+=tr("Asto.")+"\t";
     global+=tr("Base Imp.")+"\t";
     global+=tr("Tipo")+"\t";
     global+=tr("Cuota")+"\t";
     global+=tr("Tipo.RE")+"\t";
     global+=tr("R.E.")+"\t";
     global+=tr("Total")+"\t";
     global+="\n";

    QString ejercicio=ui.ejerciciocomboBox->currentText();

    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    int registros = basedatos::instancia()->num_registros_emitidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.eibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.prestserviciosuecheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.exportacionescheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         !ui.exentas_no_deduccioncheckBox->isChecked(), cuenta );

    bool filtrarfechafactura=false;
    bool filtrardoc=false;
    if (ui.ordengroupBox->isChecked())
      {
       filtrarfechafactura=ui.ordenfechafacradioButton->isChecked();
       filtrardoc=ui.ordendocradioButton->isChecked();
      }

    QSqlQuery query =
      basedatos::instancia()->registros_emitidas(ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.eibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.prestserviciosuecheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.exportacionescheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         !ui.exentas_no_deduccioncheckBox->isChecked(), cuenta , filtrarfechafactura, filtrardoc);

    ui.progressBar->setMaximum(registros);
    int numorden=0;
    int numprog=1;
    QString cadnum,guardadoc,guardaprov;
    double base=0;
    double cuota=0;
    double re=0;
    if ( query.isActive() ) {
          while ( query.next() )
               {
	        ui.progressBar->setValue(numprog);
                if (guardadoc!=query.value(0).toString() || query.value(0).toString().length()==0 
                    || numprog==1 || guardaprov!=query.value(2).toString()) numorden++;
                guardadoc=query.value(0).toString();
                guardaprov=query.value(2).toString();
	        global+= cadnum.setNum(numorden)+"\t";
	        global+= filtracad(query.value(0).toString())+"\t"; // fra.
	        global+=query.value(11).toDate().toString("dd.MM.yyyy")+"\t"; // fecha fra.
	        // stream << query.value(1).toDate().toString("dd.MM.yyyy") << "} & {\\tiny "; // fecha cont.
	        global+=query.value(2).toString()+" "; // cuenta
                global+= (query.value(13).toString().isEmpty() ?
                          filtracad(query.value(3).toString()) :
                          filtracad(query.value(13).toString()))
                        + " "; // proveedor
                global+= (query.value(14).toString().isEmpty() ?
                          cifcta(query.value(2).toString()) :
                          filtracad(query.value(14).toString())); // cif
		global+="\t";
                global+=query.value(4).toString()+"\t"; // asiento

	        global+=formateanumero(query.value(5).toDouble(),comadecimal,decimales)+"\t"; // base iva
	        global+=formateanumero(query.value(6).toDouble(),comadecimal,decimales)+"\t"; // tipo
                QString qcuota=formateanumero(query.value(5).toDouble()*
                                                 query.value(6).toDouble()/100,comadecimal,decimales);
	        global+=qcuota+"\t"; // cuota
	        global+=formateanumero(query.value(8).toDouble(),comadecimal,decimales)+"\t"; // tipo re
                QString qre=formateanumero(query.value(5).toDouble()*
                                              query.value(8).toDouble()/100,comadecimal,decimales);
	        global+=qre+"\t"; // re
                global+=formateanumero(query.value(10).toDouble(),comadecimal,decimales)+"\n"; // total
                if (query.value(16).toBool())
                   {
                    // criterio de caja
                    QSqlQuery q=basedatos::instancia()->datos_venci_apunte_iva_caja_liquidados(
                            query.value(15).toString());
                    // v.fecha_vencimiento, v.liq_iva_fecha, v.forzar_liq_iva, v.importe,
                    // v.num, v.medio_realizacion, v.cuenta_bancaria
                    bool vacio=true;
                    if (q.isActive())
                        while (q.next())
                         {
                          vacio=false;
                          global+= "\t\t";
                          global+= tr("Importe pago");
                          global+= "\t";
                          global+= formatea_redondeado_sep(q.value(3).toDouble(),comadecimal,decimales);
                          global+= "\t ";
                          global+= q.value(5).toString();
                          global+= "\t";
                          global+= tr("FECHA: ");
                          global+= "\t ";
                          global+= q.value(1).toDate().toString("dd-MM-yyyy");
                          global+= "\t";
                          global+= tr("Cuenta:");
                          global+= "\t";
                          global+= q.value(6).toString();
                          global+= "\n  ";
                         }
                    if (vacio)
                       {
                        global+= "\t\t";
                        global+= tr("Importe pago");
                        global+= "\t";
                        global+= "\t";
                        global+= "\t ";
                        global+= tr("FECHA: ");
                        global+= "\t ";
                        global+= "\t";
                        global+= tr("Cuenta:");
                        global+= "\t";
                        global+= "\n  ";
                       }
                   }
                numprog++;
	        base+=query.value(5).toDouble();
	        cuota+=convapunto(qcuota).toDouble();
	        re+=convapunto(qre).toDouble();
	      }
      }

    global+="\t\t\t\t";
   // --------------------------------------------------------------------------------------
    global+=tr("SUMAS TOTALES:")+"\t";
    global+=formateanumero(base,comadecimal,decimales)+"\t\t";
    global+=formateanumero(cuota,comadecimal,decimales)+"\t\t";
    global+=formateanumero(re,comadecimal,decimales)+"\t";
    global+=formateanumero(base+cuota+re,comadecimal,decimales);
    global+="\n";

   cb->setText(global);
   ui.progressBar->reset();
   QMessageBox::information( this, tr("Libro de Facturas"),
                            tr("Se ha pasado el contenido al portapapeles") );
}


void libro_emitidas::copiahorizontal()
{

   QClipboard *cb = QApplication::clipboard();
   QString global;


   global=filtracad(nombreempresa())+"\n\n";

   QString cadena=ui.cabeceralineEdit->text();

    // --------------------------------------------------------------------------------------
   global+=cadena+"\n\n";
    // ---------------------------------------------------------------------------------------
     global+=tr("Orden")+"\t";
     global+=tr("Factura")+"\t";
     global+=tr("Fecha Fra.")+"\t";
     global+=tr("Contab.")+ "\t";
     global+=tr("Cuenta, Cliente/deudor")+"\t";
     global+=tr("Asto.")+"\t";
     global+=tr("Base Imp.")+"\t";
     global+=tr("Tipo")+"\t";
     global+=tr("Cuota")+"\t";
     global+=tr("Tipo.RE")+"\t";
     global+=tr("R.E.")+"\t";
     global+=tr("Total")+"\t";
     global+="\n";

    QString ejercicio=ui.ejerciciocomboBox->currentText();

    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    int registros = basedatos::instancia()->num_registros_emitidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.eibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.prestserviciosuecheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.exportacionescheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         !ui.exentas_no_deduccioncheckBox->isChecked(), cuenta );

    bool filtrarfechafactura=false;
    bool filtrardoc=false;
    if (ui.ordengroupBox->isChecked())
      {
       filtrarfechafactura=ui.ordenfechafacradioButton->isChecked();
       filtrardoc=ui.ordendocradioButton->isChecked();
      }


    QSqlQuery query =
      basedatos::instancia()->registros_emitidas(ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.eibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.prestserviciosuecheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.exportacionescheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         !ui.exentas_no_deduccioncheckBox->isChecked(), cuenta, filtrarfechafactura,filtrardoc );

    ui.progressBar->setMaximum(registros);
    int numorden=0;
    int numprog=1;
    QString cadnum,guardadoc,guardaprov;
    double base=0;
    double cuota=0;
    double re=0;
    if ( query.isActive() ) {
          while ( query.next() )
               {
	        ui.progressBar->setValue(numprog);
                if (guardadoc!=query.value(0).toString() || query.value(0).toString().length()==0 
                    || numprog==1 || guardaprov!=query.value(2).toString()) numorden++;
                guardadoc=query.value(0).toString();
                guardaprov=query.value(2).toString();
	        global+= cadnum.setNum(numorden)+"\t";
	        global+= filtracad(query.value(0).toString())+"\t"; // fra.
	        global+=query.value(11).toDate().toString("dd.MM.yyyy")+"\t"; // fecha fra.
	        global+=query.value(1).toDate().toString("dd.MM.yyyy") +"\t"; // fecha cont.
	        global+=query.value(2).toString()+" "; // cuenta
                global+= (query.value(13).toString().isEmpty() ?
                          filtracad(query.value(3).toString()) :
                          filtracad(query.value(13).toString()))
                        + " "; // proveedor
                global+= (query.value(14).toString().isEmpty() ?
                          cifcta(query.value(2).toString()) :
                          filtracad(query.value(14).toString())); // cif
                global+="\t";
                global+=query.value(4).toString()+"\t"; // asiento

	        global+=formateanumero(query.value(5).toDouble(),comadecimal,decimales)+"\t"; // base iva
	        global+=formateanumero(query.value(6).toDouble(),comadecimal,decimales)+"\t"; // tipo
                QString qcuota=formateanumero(query.value(5).toDouble()*
                                                 query.value(6).toDouble()/100,comadecimal,decimales);
	        global+=qcuota+"\t"; // cuota
	        global+=formateanumero(query.value(8).toDouble(),comadecimal,decimales)+"\t"; // tipo re
                QString qre=formateanumero(query.value(5).toDouble()*
                                              query.value(8).toDouble()/100,comadecimal,decimales);
	        global+=qre+"\t"; // re
                global+=formateanumero(query.value(10).toDouble(),comadecimal,decimales)+"\n"; // total
                if (query.value(16).toBool())
                   {
                    // criterio de caja
                    QSqlQuery q=basedatos::instancia()->datos_venci_apunte_iva_caja_liquidados(
                            query.value(15).toString());
                    // v.fecha_vencimiento, v.liq_iva_fecha, v.forzar_liq_iva, v.importe,
                    // v.num, v.medio_realizacion, v.cuenta_bancaria
                    bool vacio=true;
                    if (q.isActive())
                        while (q.next())
                         {
                          vacio=false;
                          global+= "\t\t";
                          global+= tr("Importe pago");
                          global+= "\t";
                          global+= formatea_redondeado_sep(q.value(3).toDouble(),comadecimal,decimales);
                          global+= "\t ";
                          global+= q.value(5).toString();
                          global+= "\t";
                          global+= tr("FECHA: ");
                          global+= "\t ";
                          global+= q.value(1).toDate().toString("dd-MM-yyyy");
                          global+= "\t";
                          global+= tr("Cuenta:");
                          global+= "\t";
                          global+= q.value(6).toString();
                          global+= "\n  ";
                         }
                    if (vacio)
                       {
                        global+= "\t\t";
                        global+= tr("Importe pago");
                        global+= "\t";
                        global+= "\t";
                        global+= "\t ";
                        global+= tr("FECHA: ");
                        global+= "\t ";
                        global+= "\t";
                        global+= tr("Cuenta:");
                        global+= "\t";
                        global+= "\n  ";
                       }
                   }
                numprog++;
	        base+=query.value(5).toDouble();
	        cuota+=convapunto(qcuota).toDouble();
	        re+=convapunto(qre).toDouble();
	      }
      }

    global+="\t\t\t\t\t";
   // --------------------------------------------------------------------------------------
    global+=tr("SUMAS TOTALES:")+"\t";
    global+=formateanumero(base,comadecimal,decimales)+"\t\t";
    global+=formateanumero(cuota,comadecimal,decimales)+"\t\t";
    global+=formateanumero(re,comadecimal,decimales)+"\t";
    global+=formateanumero(base+cuota+re,comadecimal,decimales);
    global+="\n";
    ui.progressBar->reset();
   cb->setText(global);
   QMessageBox::information( this, tr("Libro de Facturas"),
                            tr("Se ha pasado el contenido al portapapeles") );

}


void libro_emitidas::bicheckcambiado()
{

    if (ui.bicheckBox->isChecked())
       {
        ui.horizontalradioButton->setChecked(true);
        ui.verticalradioButton->setChecked(false);
        ui.groupBox->setEnabled(false);
       }
       else
           {
            ui.horizontalradioButton->setChecked(false);
            ui.verticalradioButton->setChecked(true);
            ui.groupBox->setEnabled(true);
           }
}



void libro_emitidas::copiahorizontal_bi()
{

   QClipboard *cb = QApplication::clipboard();
   QString global;


   global=filtracad(nombreempresa())+"\n\n";

   QString cadena=ui.cabeceralineEdit->text();

    // --------------------------------------------------------------------------------------
   global+=cadena+"\n\n";
    // ---------------------------------------------------------------------------------------
     global+=tr("Orden")+"\t";
     global+=tr("Factura")+"\t";
     global+=tr("Fecha Fra.")+"\t";
     global+=tr("Contab.")+ "\t";
     global+=tr("Cuenta, Cliente/deudor")+"\t";
     global+=tr("Concepto")+"\t";
     global+=tr("Asto.")+"\t";
     global+=tr("Base Imp.")+"\t";
     global+=tr("Tipo")+"\t";
     global+=tr("Cuota")+"\t";
     global+=tr("Tipo.RE")+"\t";
     global+=tr("R.E.")+"\t";
     global+=tr("Total")+"\t";
     global+="\n";

    QString ejercicio=ui.ejerciciocomboBox->currentText();

    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    QString cuenta;
    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    int registros = basedatos::instancia()->num_registros_emitidas( ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.eibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.prestserviciosuecheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.exportacionescheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         !ui.exentas_no_deduccioncheckBox->isChecked(), cuenta );

    bool filtrarfechafactura=false;
    bool filtrardoc=false;
    if (ui.ordengroupBox->isChecked())
      {
       filtrarfechafactura=ui.ordenfechafacradioButton->isChecked();
       filtrardoc=ui.ordendocradioButton->isChecked();
      }


    QSqlQuery query =
      basedatos::instancia()->registros_emitidas(ejercicio,
         fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
         ui.interiorescheckBox->isChecked(),
         ui.aibcheckBox->isChecked(),
         ui.eibcheckBox->isChecked(),
         ui.autofacturascheckBox->isChecked(),
         ui.rectificativascheckBox->isChecked(),
         ui.autofacturasnouecheckBox->isChecked(),
         ui.prestserviciosuecheckBox->isChecked(),
         ui.seriesgroupBox->isChecked(),
         ui.serieiniciallineEdit->text(),
         ui.seriefinallineEdit->text(),
         ui.sujeciongroupBox->isChecked(),
         ui.solonosujetasradioButton->isChecked(),
         ui.excluirnosujetasradioButton->isChecked(),
         ui.exportacionescheckBox->isChecked(),
         ui.isp_op_interiorescheckBox->isChecked(),
         !ui.exentas_no_deduccioncheckBox->isChecked(), cuenta ,filtrarfechafactura,filtrardoc);

    ui.progressBar->setMaximum(registros);
    int numorden=0;
    int numprog=1;
    QString cadnum,guardadoc,guardaprov;
    double base=0;
    double cuota=0;
    double re=0;
    if ( query.isActive() ) {
          while ( query.next() )
               {
                ui.progressBar->setValue(numprog);
                if (guardadoc!=query.value(0).toString() || query.value(0).toString().length()==0
                    || numprog==1 || guardaprov!=query.value(2).toString()) numorden++;
                guardadoc=query.value(0).toString();
                guardaprov=query.value(2).toString();
                global+= cadnum.setNum(numorden)+"\t";
                global+= filtracad(query.value(0).toString())+"\t"; // fra.
                global+=query.value(11).toDate().toString("dd.MM.yyyy")+"\t"; // fecha fra.
                global+=query.value(1).toDate().toString("dd.MM.yyyy") +"\t"; // fecha cont.
                global+=query.value(2).toString()+" "; // cuenta
                global+= (query.value(13).toString().isEmpty() ?
                          filtracad(query.value(3).toString()) :
                          filtracad(query.value(13).toString()))
                        + " "; // proveedor
                global+= (query.value(14).toString().isEmpty() ?
                          cifcta(query.value(2).toString()) :
                          filtracad(query.value(14).toString())); // cif
                global+="\t";
                global+=filtracad(descripcioncuenta(query.value(12).toString()));
                global+="\t";
                global+=query.value(4).toString()+"\t"; // asiento

                global+=formateanumero(query.value(5).toDouble(),comadecimal,decimales)+"\t"; // base iva
                global+=formateanumero(query.value(6).toDouble(),comadecimal,decimales)+"\t"; // tipo
                QString qcuota=formateanumero(query.value(5).toDouble()*
                                                 query.value(6).toDouble()/100,comadecimal,decimales);
                global+=qcuota+"\t"; // cuota
                global+=formateanumero(query.value(8).toDouble(),comadecimal,decimales)+"\t"; // tipo re
                QString qre=formateanumero(query.value(5).toDouble()*
                                              query.value(8).toDouble()/100,comadecimal,decimales);
                global+=qre+"\t"; // re
                global+=formateanumero(query.value(10).toDouble(),comadecimal,decimales)+"\n"; // total
                numprog++;
                base+=query.value(5).toDouble();
                cuota+=convapunto(qcuota).toDouble();
                re+=convapunto(qre).toDouble();
              }
      }

    global+="\t\t\t\t\t\t";
   // --------------------------------------------------------------------------------------
    global+=tr("SUMAS TOTALES:")+"\t";
    global+=formateanumero(base,comadecimal,decimales)+"\t\t";
    global+=formateanumero(cuota,comadecimal,decimales)+"\t\t";
    global+=formateanumero(re,comadecimal,decimales)+"\t";
    global+=formateanumero(base+cuota+re,comadecimal,decimales);
    global+="\n";

   ui.progressBar->reset();

   cb->setText(global);
   QMessageBox::information( this, tr("Libro de Facturas"),
                            tr("Se ha pasado el contenido al portapapeles") );
}

void libro_emitidas::latex()
{
    if (!ui.horizontalradioButton->isChecked())
      {
       if  (!generalatexrepercutido())
          return;
      }

    if (ui.horizontalradioButton->isChecked())
      {
       if (ui.bicheckBox->isChecked())
        {
           if  (!generalatexrepercutido_horizontal_bi())
              return;
        }
        else
           {
            if  (!generalatexrepercutido_horizontal())
             return;
           }
      }

    int valor=editalatex(nombrefichero());
    if (valor==1)
        QMessageBox::warning( this, tr("LIBRO FACTURAS"),tr("PROBLEMAS al llamar al editor Latex"));

}


void libro_emitidas::tabla()
{
    QString ejercicio=ui.ejerciciocomboBox->currentText();

    bool fechacontable=false;
    if (ui.fechasgroupBox->isChecked())
       {
        ejercicio.clear();
        if (ui.fcontableradioButton->isChecked()) fechacontable=true;
       }

    QString cuenta;

    if (ui.cuentagroupBox->isChecked()) cuenta=ui.cuentalineEdit->text();

    tabla_iva_rep *t = new tabla_iva_rep(usuario, comadecimal, decimales);
    t->pasa_info(ejercicio,
                 fechacontable,ui.inicialdateEdit->date(),ui.finaldateEdit->date(),
                 ui.interiorescheckBox->isChecked(),
                 ui.aibcheckBox->isChecked(),
                 ui.eibcheckBox->isChecked(),
                 ui.autofacturascheckBox->isChecked(),
                 ui.rectificativascheckBox->isChecked(),
                 ui.autofacturasnouecheckBox->isChecked(),
                 ui.prestserviciosuecheckBox->isChecked(),
                 ui.seriesgroupBox->isChecked(),
                 ui.serieiniciallineEdit->text(),
                 ui.seriefinallineEdit->text(),
                 ui.sujeciongroupBox->isChecked(),
                 ui.solonosujetasradioButton->isChecked(),
                 ui.excluirnosujetasradioButton->isChecked(),
                 ui.exportacionescheckBox->isChecked(),
                 ui.isp_op_interiorescheckBox->isChecked(),
                 ! ui.exentas_no_deduccioncheckBox->isChecked(),cuenta);
    t->exec();
    delete(t);

}


void libro_emitidas::cuenta_finedicion()
{
    ui.cuentalineEdit->setText(expandepunto(ui.cuentalineEdit->text(),anchocuentas()));
}


void libro_emitidas::boton_cuenta_pulsado()
{
    QString cadena2;
    buscasubcuenta *labusqueda=new buscasubcuenta(ui.cuentalineEdit->text());
    int cod=labusqueda->exec();
    cadena2=labusqueda->seleccioncuenta();
    if (cod==QDialog::Accepted && existesubcuenta(cadena2))
      ui.cuentalineEdit->setText(cadena2);
       else ui.cuentalineEdit->setText("");
    delete labusqueda;
}


void libro_emitidas::cuenta_cambiada()
{
 QString cadena;
 if (existecodigoplan(ui.cuentalineEdit->text(),&cadena))
    ui.descripcuentalabel->setText(cadena);
  else
    ui.descripcuentalabel->setText("");
}
