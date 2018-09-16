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

#include "notas.h"
#include <QSqlQuery>
#include "funciones.h"
#include "basedatos.h"
#include <QMessageBox>

notas::notas(bool cargar) : QDialog() {
    ui.setupUi(this);


connect(ui.aceptarpushButton,SIGNAL(clicked()),SLOT(botonaceptar()));

connect(ui.imprimirpushButton,SIGNAL(clicked()),SLOT(imprimir()));

connect(ui.texpushButton,SIGNAL(clicked()),SLOT(editlatex()));

connect(ui.copiarpushButton, SIGNAL(clicked()), SLOT(copiar()));

modoconsulta=false;

#ifdef NOEDITTEX
 ui.texpushButton->hide();
#endif

// cargamos contenido
 if (cargar)
 {
  QString contenido=basedatos::instancia()->notas();
  ui.textEdit->setText(contenido);
 }
}

void notas::pasa_contenido(QString contenido)
{
  ui.textEdit->setText(contenido);
}

void notas::activa_modoconsulta()
{
    modoconsulta=true;
    ui.textEdit->setReadOnly(true);

}

void notas::botonaceptar()
{
    if (!modoconsulta)
    // grabamos contenido
     {
      QString contenido=ui.textEdit->toPlainText();
      basedatos::instancia()->grabarnotas(contenido);
     }
    accept();
}

void notas::copiar()
{
    QClipboard *cb = QApplication::clipboard();
    cb->setText(ui.textEdit->toPlainText());
    QMessageBox::information( this, tr("Notas de empresa"),
                              tr("Se ha pasado el contenido al portapapeles") );

}

void notas::generalatex()
{
    QString qfichero=dirtrabajo();
    qfichero.append(QDir::separator());
    qfichero+="notas.tex";
    // QMessageBox::warning( this, tr("Estados Contables"),qfichero);
     QFile fichero(adapta(qfichero));
     if (! fichero.open( QIODevice::WriteOnly ) ) return;
     QTextStream stream( &fichero );
     stream.setCodec("UTF-8");

     stream << cabeceralatex();
     // stream << margen_extra_latex();

     QString cabecerapagina="";
     cabecerapagina+= "\\begin{flushleft}"; cabecerapagina+="\n";
     cabecerapagina+= "\\today"; cabecerapagina+= "\n";
     cabecerapagina+= "\\newline"; cabecerapagina+="\n";
     cabecerapagina+= "\\end{flushleft}"; cabecerapagina+="\n";

     cabecerapagina+= "\\begin{center}"; cabecerapagina+= "\n";
     cabecerapagina+= "{\\huge \\textbf {";  cabecerapagina+="NOTAS DE LA EMPRESA";
              cabecerapagina+="}}"; cabecerapagina+="\n";
     cabecerapagina+= "\\end{center}"; cabecerapagina+= "\n";
     cabecerapagina+= "\n";

     cabecerapagina+= "\\begin{center}"; cabecerapagina+= "\n";
     cabecerapagina+= "{\\Large \\textbf {";  cabecerapagina+=filtracad(nombreempresa());
              cabecerapagina+="}}"; cabecerapagina+="\n";
     cabecerapagina+= "\\end{center}"; cabecerapagina+= "\n";
     cabecerapagina+= "\n";

     stream << cabecerapagina;

     stream << ui.textEdit->toPlainText();

     // imprimimos final del documento latex
     stream << "% FIN_CUERPO" << "\n";
     stream << "\\end{document}" << "\n";

     fichero.close();

}


void notas::imprimir()
{
    generalatex();
    int valor=imprimelatex("notas");
    if (valor==1)
        QMessageBox::warning( this, tr("IMPRIMIR NOTAS"),tr("PROBLEMAS al llamar a Latex"));
    if (valor==2)
        QMessageBox::warning( this, tr("IMPRIMIR NOTAS"),
                                 tr("PROBLEMAS al llamar a 'dvips'"));
    if (valor==3)
        QMessageBox::warning( this, tr("IMPRIMIR NOTAS"),
                              tr("PROBLEMAS al llamar a ")+programa_imprimir());

}

void notas::editlatex()
{
  generalatex();
  int valor=editalatex("notas");
  if (valor==1)
      QMessageBox::warning( this, tr("LATEX - NOTAS"),tr("PROBLEMAS al llamar al editor Latex"));
}

void notas::cambia_titulo(QString titulo)
{
    setWindowTitle(titulo);
}
