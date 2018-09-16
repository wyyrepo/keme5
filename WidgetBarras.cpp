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

#include "WidgetBarras.h"
#include "funciones.h"
#include "directorio.h"
#include <QGraphicsRectItem>
#include <QList>
#include <QListIterator>
#include <QFileDialog>
#include <QMessageBox>

// funcion maximo
#define MAX(a,b) a < b ? b : a

// numero de colores
#define NCOLORES 15

// posición del scene
#define SCENEXI 9
#define SCENEYI 9
#define SCENEXF 582
#define SCENEYF 382

// posición etiquetas iniciales
#define ERRORY 200
#define CENTROX 300
#define NOMBREEMPRESAY 20
#define EJERCICIOY 50
#define TITULO1X 250
#define TITULO2X 350
#define TITULOY 100

// posición de rectángulos
#define MAXIMOY 350
#define MINIMOY 150
#define PARTEIZQUIERDAX 200
#define PARTEDERECHAX 300
#define PARTEIDY MINIMOY
#define ANCHORECTANGULO 100

// posición de etiquetas (leyenda de rectángulos)
#define POSICIONINICIALETIQUETASY 160
#define SEPARACIONETIQUETASY 20
#define POSICIONETIQUETAS1X 20
#define POSICIONETIQUETAS2X 420

// tamaños de guardar dibujo
#define INICIODIBUJOX 0
#define INICIODIBUJOY 0
#define TAMANODIBUJOX 600
#define TAMANODIBUJOY 400

WidgetBarras::WidgetBarras (QString nombreEmpresa,QString ejercicio,
                            QString titulo1,QString titulo2,
                            QStringList etiquetas_parte1,
                            QStringList etiquetas_parte2,
                            QList<double> valores_parte1,
                            QList<double> valores_parte2,
			                QWidget * parent) : QWidget (parent) {
    ui.setupUi(this);

    graphicsscene = new QGraphicsScene(SCENEXI,SCENEYI,SCENEXF,SCENEYF);
    ui.graphicsView->setScene(graphicsscene);

    double total1=0,total2=0;
    bool negativo = false; // indica si algún valor es negativo

    // comprueba la validez de los valores_parteX introducidos
    comprobarvalidez(valores_parte1,&total1,&negativo);
    comprobarvalidez(valores_parte2,&total2,&negativo);

    // comprueba que los datos de activo y pasivo sean mayores que 0 y ninguno de los datos negativos, si no, no genera gráfico
    if ( (total1!=0) && (total2!=0) && (!negativo) ) {
        mostrar(nombreEmpresa,ejercicio,
                titulo1,titulo2,
                etiquetas_parte1,
                etiquetas_parte2,
                valores_parte1,
                valores_parte2);
    }
    else {
        etiquetasbasicas(nombreEmpresa,ejercicio);

        QGraphicsTextItem *elemento;
        elemento = new QGraphicsTextItem;
        // si hay algún elemento negativo, sino el activo, pasivo o ambos valen 0
        if (negativo) { elemento = graphicsscene->addText(""); elemento->setHtml("Alg&uacute;n elemento es negativo"); }
        else { elemento = graphicsscene->addText( "El activo, el pasivo o ambos valen 0" ); }
        elemento->setPos(CENTROX-(elemento->boundingRect().width()/2),ERRORY);
    }

    connect( ui.guardarpushButton, SIGNAL(clicked()), this, SLOT (guardarimagen()) );
    connect(ui.salirpushButton, SIGNAL(clicked()), parent, SLOT(close()));
}

// comprueba la validez de los valores_parteX introducidos
void WidgetBarras::comprobarvalidez(QList<double> valores,double *total,bool *negativo) {
    double valor;
    QListIterator<double> i( valores );
    while (i.hasNext()) {
        valor = i.next();
        if (valor<0) { (*negativo) = true; }
        (*total) += valor;
    }
}


WidgetBarras::~WidgetBarras()
{
    QListIterator<QGraphicsItem*> i( graphicsscene->items() );
    while (i.hasNext())
        delete i.next();
    delete graphicsscene;
}


// Genera todos los elementos de la ventana
void WidgetBarras::mostrar(QString nombreEmpresa,QString ejercicio,
                          QString titulo1,QString titulo2,
                          QStringList etiquetas_parte1,
                          QStringList etiquetas_parte2,
                          QList<double> valores_parte1,
                          QList<double> valores_parte2) {
    
    QGraphicsTextItem *elemento;


    // Establece el nombre de empresa y ejercicio
    etiquetasbasicas(nombreEmpresa,ejercicio);

    // Establece los títulos
    elemento = new QGraphicsTextItem;
    elemento = graphicsscene->addText( titulo1 );
    elemento->setPos(TITULO1X-(elemento->boundingRect().width()/2),TITULOY);
    elemento = new QGraphicsTextItem;
    elemento = graphicsscene->addText( titulo2 );
    elemento->setPos(TITULO2X-(elemento->boundingRect().width()/2),TITULOY);

    // Total de capital
    int i;
    double total = 0,total2 = 0; // valor de la suma de los valores (total de activo/pasivo)
    for (i=0;i<valores_parte1.size();i++) {
        total +=valores_parte1.at(i);
    }
    for (i=0;i<valores_parte2.size();i++) {
        total2 +=valores_parte2.at(i);
    }
    total = MAX (total,total2);

    // Dibujar parte izquierda
    dibujarcolumna(etiquetas_parte1, valores_parte1, PARTEIZQUIERDAX, total, POSICIONETIQUETAS1X, 0);

    // Dibujar parte derecha
    dibujarcolumna(etiquetas_parte2, valores_parte2, PARTEDERECHAX, total, POSICIONETIQUETAS2X, valores_parte1.size() );
}


// Dibuja la columna y las etiquetas
void WidgetBarras::dibujarcolumna(QStringList etiquetas, QList<double> valores, int posicionx, double total, int posicionetiquetasx, int desplazamiento) {
    QGraphicsRectItem *itemrectangulo;
    QGraphicsTextItem *itemtexto;

    double px,py;
    int i,j;
    for (i=0;i<valores.size();i++) {

        // establece la posición del rectángulo
        px=posicionx;
        py=PARTEIDY;
        for (j=0;j<i;j++) {
            py+=(valores.at(j)/total)*(MAXIMOY-MINIMOY);
        }

        // dibuja el rectángulo
        itemrectangulo = new QGraphicsRectItem(px,py,ANCHORECTANGULO,(valores.at(i)/total)*(MAXIMOY-MINIMOY));
        graphicsscene->addItem( itemrectangulo );        
        establecercaja ( itemrectangulo , i + desplazamiento );

        // dibuja la etiqueta
        itemtexto = new QGraphicsTextItem(etiquetas.at(i));
        graphicsscene->addItem(itemtexto);
        establecertexto ( itemtexto , posicionetiquetasx , POSICIONINICIALETIQUETASY+i*SEPARACIONETIQUETASY , i + desplazamiento );
    }

}

// Pinta la etiqueta de empresa y ejercicio
void WidgetBarras::etiquetasbasicas(QString nombreEmpresa, QString ejercicio) {
    QGraphicsTextItem *elemento;
    // Establece el nombre de la empresa
    elemento = new QGraphicsTextItem;
    elemento = graphicsscene->addText( nombreEmpresa );
    elemento->setHtml(tr("<b> %1 </b>").arg(nombreEmpresa));
    elemento->setPos(CENTROX-(elemento->boundingRect().width()/2),NOMBREEMPRESAY);

    // Establece el nombre del ejercicio
    elemento = new QGraphicsTextItem;
    elemento = graphicsscene->addText( tr("EJERCICIO %1").arg(ejercicio) );
    elemento->setPos(CENTROX-(elemento->boundingRect().width()/2),EJERCICIOY);
}


// Pinta un rectángulo de un color
void WidgetBarras::establecercaja(QGraphicsRectItem * caja,int i) {
    i = i % NCOLORES;
    switch (i) {
        case 0:  caja->setBrush(Qt::green); break;
        case 1:  caja->setBrush(Qt::blue); break;
        case 2:  caja->setBrush(Qt::red); break;
        case 3:  caja->setBrush(Qt::gray); break;
        case 4:  caja->setBrush(Qt::cyan); break;
        case 5:  caja->setBrush(Qt::black); break;
        case 6:  caja->setBrush(Qt::darkCyan); break;
        case 7:  caja->setBrush(Qt::darkGreen); break;
        case 8:  caja->setBrush(Qt::darkBlue); break;
        case 9:  caja->setBrush(Qt::darkRed); break;
        case 10: caja->setBrush(Qt::darkYellow); break;
        case 11: caja->setBrush(Qt::darkGray); break;
        case 12: caja->setBrush(Qt::magenta); break;
        case 13: caja->setBrush(Qt::lightGray); break;
        case 14: caja->setBrush(Qt::darkMagenta); break;
    }
}

// Pinta las letras de un color y las posiciona
void WidgetBarras::establecertexto(QGraphicsTextItem * texto,int x, int y, int i) {
    texto->setPos(x,y);
    i = i % NCOLORES;
    switch (i) {
        case 0:  texto->setDefaultTextColor(Qt::green); break;
        case 1:  texto->setDefaultTextColor(Qt::blue); break;
        case 2:  texto->setDefaultTextColor(Qt::red); break;
        case 3:  texto->setDefaultTextColor(Qt::gray); break;
        case 4:  texto->setDefaultTextColor(Qt::cyan); break;
        case 5:  texto->setDefaultTextColor(Qt::black); break;
        case 6:  texto->setDefaultTextColor(Qt::darkCyan); break;
        case 7:  texto->setDefaultTextColor(Qt::darkGreen); break;
        case 8:  texto->setDefaultTextColor(Qt::darkBlue); break;
        case 9:  texto->setDefaultTextColor(Qt::darkRed); break;
        case 10: texto->setDefaultTextColor(Qt::darkYellow); break;
        case 11: texto->setDefaultTextColor(Qt::darkGray); break;
        case 12: texto->setDefaultTextColor(Qt::magenta); break;
        case 13: texto->setDefaultTextColor(Qt::lightGray); break;
        case 14: texto->setDefaultTextColor(Qt::darkMagenta); break;
    }
}


// Salva la imagen creada en un archivo
void WidgetBarras::guardarimagen() {
    QString ruta;

#ifdef NOMACHINE
  directorio *dir = new directorio();
  dir->pasa_directorio(adapta(dirtrabajobd()));
  dir->filtrar("*.png");
  dir->activa_pide_archivo("");
  if (dir->exec() == QDialog::Accepted)
    {
      ruta=dir->nuevo_conruta();
    }
   delete(dir);

#else

    // Selección del nombre del fichero
    QFileDialog dialogofich(this);
    dialogofich.setFileMode(QFileDialog::AnyFile);
    dialogofich.setAcceptMode (QFileDialog::AcceptOpen );
    dialogofich.setLabelText ( QFileDialog::LookIn, tr("Directorio:") );
    dialogofich.setLabelText ( QFileDialog::FileName, tr("Archivo:") );
    dialogofich.setLabelText ( QFileDialog::FileType, tr("Tipo de archivo:") );
    dialogofich.setLabelText ( QFileDialog::Accept, tr("Guardar") );
    dialogofich.setLabelText ( QFileDialog::Reject, tr("Cancelar") );

    QStringList filtros;
    filtros << tr("Archivos de imagen (*.png)");
    dialogofich.setNameFilters(filtros);
    dialogofich.setDirectory(adapta(dirtrabajo()));
    dialogofich.setWindowTitle(tr("GUARDAR GRÁFICO"));

    // Si se ha seleccionado guardar
    if (dialogofich.exec()) {

        // Ruta del archivo seleccionado        
        QStringList fileNames;
        fileNames = dialogofich.selectedFiles();
        ruta=fileNames.at(0);
    }

#endif

    if (ruta.isEmpty()) return;

    if (!ruta.endsWith(".png")) ruta += ".png";

    // rectangulo de fondo
    QGraphicsRectItem *rectangulofondo = new QGraphicsRectItem(INICIODIBUJOX, INICIODIBUJOX, TAMANODIBUJOX, TAMANODIBUJOY);
    graphicsscene->addItem( rectangulofondo );
    rectangulofondo->setBrush(Qt::white);
    rectangulofondo->setZValue(-1);

    // genera la imagen
    QImage *imagen = new QImage(TAMANODIBUJOX, TAMANODIBUJOY, QImage::Format_RGB32);
    QPainter *p = new QPainter(imagen);
    p->setRenderHint(QPainter::Antialiasing);
    graphicsscene->render(p);
    p->end();

    // guarda la imagen
    bool grabado = imagen->save(adapta(ruta), "PNG");

    // elimina los punteros
    delete imagen;
    delete p;
    delete rectangulofondo;

    // mensaje de confirmación /rechazo de guardado
    if (grabado) { QMessageBox::information ( this , tr("Gráfico"), tr("Imagen guardada en \n%1").arg(ruta)); }
    else { QMessageBox::warning ( this , tr("Gráfico"), tr("Ha habido problemas al intentar guardar la imagen en \n%1").arg(ruta)); }

}

