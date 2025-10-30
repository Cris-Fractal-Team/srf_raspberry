

#include <cmath>
#include "lib/hailolib/DetectorCaraHaloScrfd.h"
#include "lib/utils/NumberUtils.h"
#include "lib/utils/GStringUtils.h"

/**
 * Retorna el acho de la cara
 */
int DeteccionCaraHailo::getAncho()
{
    return ptoInfDer.x - ptoSupIzq.x + 1;
}

/**
 * Retorna la altura de la cara
 */
int DeteccionCaraHailo::getAltura()
{
    return ptoInfDer.y - ptoSupIzq.y + 1;
}

/**
 * Area del a deteccion
 */
int DeteccionCaraHailo::getArea()
{
    return getAncho() * getAltura();
}

/**
 * Retorna el punto central del area detectada
 */
PuntoHailo DeteccionCaraHailo::getCentro()
{
    PuntoHailo punto;

    punto.x = (ptoSupIzq.x + ptoInfDer.x)/2;
    punto.y = (ptoSupIzq.y + ptoInfDer.y)/2;

    return punto;
}

/**
 * Agrega un valor a las coordenadas x y otro a las coordenadas y
 * de todos los puntos de la cara
 */
void DeteccionCaraHailo::desplazaPtosCara( float deltaX, float deltaY )
{
    ojoIzq.x+= deltaX;
    ojoDer.x+= deltaX;
    nariz.x+= deltaX;
    bocaIzq.x+= deltaX;
    bocaDer.x+= deltaX;

    ojoIzq.y+= deltaY;
    ojoDer.y+= deltaY;
    nariz.y+= deltaY;
    bocaIzq.y+= deltaY;
    bocaDer.y+= deltaY;
}


/**
 * Agrega un valor a las coordenadas x y otro a las coordenadas y
 * de los puntos de la region de detecion
 */
void DeteccionCaraHailo::desplazaRegion( float deltaX, float deltaY )
{
    ptoSupIzq.x+= deltaX;
    ptoSupIzq.y+= deltaY;
    ptoInfDer.x+= deltaX;
    ptoInfDer.y+= deltaY;
}


/**
 * Valida si un punto (x,y) esta dentro de la deteccion
 */
bool DeteccionCaraHailo::contienePunto( int x, int y )
{
    if ((( x >= ptoSupIzq.x ) && ( x <= ptoInfDer.x )) && (( y >= ptoSupIzq.y ) && ( y <= ptoInfDer.y )))
    {
        return true;
    }
    return false;
}

/**
 * Calcula la intereccion sobre la union de dos detecciones
 */
float DeteccionCaraHailo::calcularIoU( DeteccionCaraHailo *det )
{
    // Coordenadas de la intersección
    float xLeft   = std::max(ptoSupIzq.x, det->ptoSupIzq.x);
    float yTop    = std::max(ptoSupIzq.y, det->ptoSupIzq.y);
    float xRight  = std::min(ptoInfDer.x, det->ptoInfDer.x);
    float yBottom = std::min(ptoInfDer.y, det->ptoInfDer.y);

    // Si no hay intersección
    if (xRight < xLeft || yBottom < yTop)
        return 0.0f;

    int interArea = (xRight - xLeft) * (yBottom - yTop);

    int areaA = getAncho() * getAltura();
    int areaB = getAncho() * getAltura();

    float iou = static_cast<float>(interArea) / (areaA + areaB - interArea);
    return iou;
}

/**
 * Valida si al cara esta de perfil
 */
bool DeteccionCaraHailo::caraDePerfil()
{
    float minX,maxX;
    float puntoMedioOjos = (ojoIzq.x + ojoDer.x )/2;
    float deltaOjos = abs(ojoIzq.x - ojoDer.x)/4;
    
    minX = ojoIzq.x - deltaOjos;
    maxX = ojoDer.x + deltaOjos;

    if ( ( nariz.x >= minX ) && ( nariz.x <= maxX ))
        return false;

    return true;
}

/**
 * Valida si al cara esta de perfil.
 * No se analiza si la cara esta mirando hacia arriba o hacia abajo
 */
bool DeteccionCaraHailo::caraFrontal( std::vector<cv::Point2f> *lstPuntosReferencia, cv::Mat transAlineacion, float afinidadMaxima )
{
    // float minX,maxX;
    // float puntoMedioOjos = (ojoIzq.x + ojoDer.x )/2;
    // float puntoMedioBoca = (bocaIzq.x + bocaDer.x )/2;    
    
    // float deltaOjos = abs(ojoIzq.x - ojoDer.x)/4;
    // float deltaBoca = abs(bocaIzq.x - bocaDer.x)/4;
        
    // float puntoMedio = getAncho()/2;
    // float deltaCentroOjos = puntoMedio-puntoMedioOjos;
    // float deltaCentroBoca = puntoMedio-puntoMedioBoca;
    // float deltaCentro = puntoMedio / 4;
    // float deltaCentroNariz = nariz.x - puntoMedio;

    // // valida si el centro de los ojos no esta muy desviado del centro de la cara
    // if ( abs(deltaCentroOjos) > deltaCentro )
    //     return false;

    // // valida si el centro de la boca no esta muy desviado del centro de la cara
    // if ( abs(deltaCentroOjos) > deltaCentro )
    //     return false;
    
    // // valida si la nariz no esta muy desviada del centro de la cara
    // // if ( abs(deltaCentroNariz) > (puntoMedio/3) )
    // //     return false;

    // // valida que la nariz no este muy desviada del centro de los ojos
    // minX = ojoIzq.x - deltaOjos;
    // maxX = ojoDer.x + deltaOjos;

    // if ( ( nariz.x < minX ) || ( nariz.x > maxX ))
    //     return false;

    // // valida que la nariz no este muy desviada del centro de la boca
    // minX = bocaIzq.x - deltaOjos;
    // maxX = bocaDer.x + deltaOjos;
    
    // if ( ( nariz.x < minX ) || ( nariz.x > maxX ))    
    //     return false;
    
    // return true;

    std::vector<cv::Point2f> lstPuntos;
    cv::Point2f p, pRef;

    lstPuntos.push_back(cv::Point2f(ojoIzq.x, ojoIzq.y));
    lstPuntos.push_back(cv::Point2f(ojoDer.x, ojoDer.y));
    lstPuntos.push_back(cv::Point2f(nariz.x, nariz.y));
    lstPuntos.push_back(cv::Point2f(bocaIzq.x, bocaIzq.y));
    lstPuntos.push_back(cv::Point2f(bocaDer.x, bocaDer.y));

    double se = 0.0;

    for ( int i=0; i<5; i++)
    {
        p = lstPuntos[i];
        pRef = lstPuntosReferencia->at(i);

        double x = transAlineacion.at<double>(0,0)*p.x + transAlineacion.at<double>(0,1)*p.y + transAlineacion.at<double>(0,2);
        double y = transAlineacion.at<double>(1,0)*p.x + transAlineacion.at<double>(1,1)*p.y + transAlineacion.at<double>(1,2);
        double dx = x - pRef.x;
        double dy = y - pRef.y;
        se += dx*dx + dy*dy;
    }

    double afinidad = std::sqrt(se/5.0);

    // cout << "Afinidad Frontal: " << afinidad << " VS " << afinidadMaxima <<  endl;

    if ( afinidad > afinidadMaxima )
        return false;

    return true;
}


/**
 * Retorna el bounding box o caja que redea a la deteccion
 * con formato soportado por opencv
 */
cv::Rect DeteccionCaraHailo::getOpenCV2DRectBoundingBox()
{
    Rect rpta = Rect(ptoSupIzq.x, ptoSupIzq.y, getAncho(), getAltura());
    return rpta;
}


/**
 * Ajusta las cooredanas de la deteccion para que sea un cuadrado
 * tomando como base la dimension mayor del rostro
 * 
 *  param anchoMax : ancho de la imagen en la que se hizo la deteccion
 * 
 *  param alturaMax : altura de la imagen en la que se hizo la deteccion
 */
void DeteccionCaraHailo::ajustarCuadrado( int anchoMax, int alturaMax )
{
    int ancho = getAncho();
    int altura = getAltura();
    int delta,deltaMed;
    
    if ( ancho > altura )
    {
        delta = ancho-altura;
        deltaMed = delta / 2;

        ptoSupIzq.y -= deltaMed;
        ptoInfDer.y += deltaMed;

        if ( ptoSupIzq.y < 0 ) 
        {
            ptoInfDer.y+= -ptoSupIzq.y;
            ptoSupIzq.y = 0;
        }
        else
        if ( ptoInfDer.y >= alturaMax )
        {
            delta = ptoInfDer.y - alturaMax + 1;
            ptoSupIzq.y -= delta;
            ptoInfDer.y -= delta;
        }
    }
    else
    if ( altura > ancho )
    {
        delta = altura-ancho;
        deltaMed = delta / 2;

        ptoSupIzq.x -= deltaMed;
        ptoInfDer.x += deltaMed;

        if ( ptoSupIzq.x < 0 ) 
        {
            ptoInfDer.x+= -ptoSupIzq.x;
            ptoSupIzq.x = 0;
        }
        else
        if ( ptoInfDer.x >= anchoMax )
        {
            delta = ptoInfDer.x - anchoMax + 1;
            ptoSupIzq.x -= delta;
            ptoInfDer.x -= delta;
        }
    }
}

/**
 * Ajusta las cooredanas de la deteccion para que sea un cuadrado
 * tomando como base la dimension minima
 * 
 *  param anchoMax : ancho de la imagen en la que se hizo la deteccion
 * 
 *  param alturaMax : altura de la imagen en la que se hizo la deteccion
 */
void DeteccionCaraHailo::ajustarMiniCuadrado( int anchoMax, int alturaMax )
{
    int ancho = getAncho();
    int altura = getAltura();
    int delta,deltaMed;
    
    if ( ancho > altura )
    {
        delta = ancho-altura;
        deltaMed = delta / 2;

        ptoSupIzq.x += deltaMed;
        ptoInfDer.x -= deltaMed;        
    }
    else
    if ( altura > ancho )
    {
        delta = altura-ancho;
        deltaMed = delta / 2;

        ptoSupIzq.y += deltaMed;
        ptoInfDer.y -= deltaMed;
    }
}


/**
 * Constructor
 */
DetectorCarasHailoSCRFD::DetectorCarasHailoSCRFD()
{
    nms_iou_thresh = 0.6;

    previsualizaImgParaDeteccion = false;
    esperarPrevImgParaDeteccion = false;
}


/**
 * Dimensiones de las imagenes que acepta la red
 */
void DetectorCarasHailoSCRFD::setDimImagenes( int ancho, int altura )
{
    imgModeloAltura = altura;
    imgModeloAncho = ancho;
}

/**
 * Carga el modelo
 */
int DetectorCarasHailoSCRFD::cargarModelo( string path )
{
    int resultado = runner.cargarRed(path);

    if ( resultado == 0 )
    {
        char *pos;
        hailo_vstream_info_t infoVstr;
        std::vector<hailo_vstream_info_t> lstInfo;

        lstInfo = runner.getOutVStreamsInfo();            
        infoVstr = lstInfo.at(0);

        pos = strstr(infoVstr.name, "2_5g");
        cout << "1ra capa de salida: " << infoVstr.name << " : " << pos << endl;
        
        if ( strstr(infoVstr.name,"500m") != NULL )
        {
            tipoModelo = _DetectorCarasHailoSCRFD_500m_;
        }
        else
        if ( strstr(infoVstr.name,"2_5g") != NULL )
        {
            tipoModelo = _DetectorCarasHailoSCRFD_2_5g_;
        }
        else    
        if ( strstr(infoVstr.name,"10g") != NULL )
        {
            tipoModelo = _DetectorCarasHailoSCRFD_10g_;
        }
        else 
        {
            tipoModelo = 0;
        }

        cout << "Tipo de modelo SCRFD detectado : " << tipoModelo << endl;
    }
    else
    {
        tipoModelo = 0;
    }

    return resultado;
}

/**
 * Ejecuta la deteccion
 */
bool DetectorCarasHailoSCRFD::ejecutar( cv::Mat imagen )
{
    int deltaBorde,tecla;

    // auto inicio = std::chrono::high_resolution_clock::now();
    anchoImgOrig = imagen.cols;
    alturaImgOrig = imagen.rows;

    // hacemos que la imagen sea cuadrada
    if ( anchoImgOrig > alturaImgOrig )
    {        
        deltaBorde = (anchoImgOrig - alturaImgOrig) / 2;
        cv::Mat cuadrada(anchoImgOrig, anchoImgOrig, CV_8UC3, cv::Scalar(127, 127, 127));
        cv::Mat roi = cuadrada(cv::Rect(0, deltaBorde, anchoImgOrig, alturaImgOrig));
        imagen.copyTo(roi);
        imagen = cuadrada;
    }
    else
    if ( alturaImgOrig > anchoImgOrig )
    {
        deltaBorde = (alturaImgOrig - anchoImgOrig ) / 2;
        cv::Mat cuadrada(alturaImgOrig, alturaImgOrig, CV_8UC3, cv::Scalar(0, 0, 0));
        cv::Mat roi = cuadrada(cv::Rect(deltaBorde, 0, anchoImgOrig, alturaImgOrig));
        imagen.copyTo(roi);
        imagen = cuadrada;
    }
    
    // redimensiona la imagen a la resolucion de entrada
    cv::resize(imagen, imagenRedim, cv::Size(imgModeloAncho,imgModeloAltura));   

    // muestra la imagen que se envia al detector de rostros
    if ( previsualizaImgParaDeteccion == true )
    {
        cv::imshow("Cuadro Deteccion", imagenRedim);
        if ( esperarPrevImgParaDeteccion == true )
        {
            tecla = cv::waitKey(3);
            if ( tecla == 'p' )
            {
                // se guarda la imagen para usarla luego
                std::vector<int> parametros;
                string pathFoto = "./fotos/" + prefijoImgDeteccion;
                pathFoto.append(GStringUtils::to_fixed_digits(secuencialImgDeteccion,6));
                pathFoto.append(".png");
                secuencialImgDeteccion++;

                parametros.push_back(cv::IMWRITE_PNG_COMPRESSION);
                parametros.push_back(3); 
                cout << "Guardando foto para deteccion :" << pathFoto << endl;
                cv::imwrite(pathFoto, imagenRedim, parametros);
    }
        }
    }

    // convierte la imagen a RGB
    cv::Mat imagenRgb;
    cv::cvtColor(imagenRedim, imagenRgb, cv::COLOR_BGR2RGB);

    
    // crea el buffer para ingresar la imagen
    std::vector<uint8_t> input_buffer(imagenRgb.total() * imagenRgb.elemSize());
    std::memcpy(input_buffer.data(), imagenRgb.data, input_buffer.size());

    // auto fin = std::chrono::high_resolution_clock::now(); // Finaliza el cronómetro
    // auto duracion = std::chrono::duration_cast<std::chrono::microseconds>(fin - inicio);
    // std::cout << "* Tiempo Procesa Imagen para inferencia : " << duracion.count() << " us\n";

    if ( runner.ejecutarInferencia(input_buffer.data(), input_buffer.size()) == false ) 
        return false;    

    return true;
}

/**
 * Extrae las cajas que rodean a las detecciones
 * 
 *      lstDet : 
 *          Lista en la que se guardan las detecciones
 * 
 *      nomCapaCajas :
 *          Nombre de la capa de salida que tiene las cajas
 * 
 *      nomCapaConf :
 *          Nombre de la capa de salida con las confidencias
 * 
 *      anchoImg : 
 *          Ancho de la imagen original
 * 
 *      alturaImg : 
 *          Altura de la imagen original
 * 
 *      confMin : 
 *          Es el valor minimo de la confianza para retornar la deteccion
 * 
 *      factorEscalaXImg :
 *          Factor por el que se escalan las coordenadas X para que coincidan con las dimensiones
 *          de la imagen inicial
 *
 *      factorEscalaYImg :
 *          Factor por el que se escalan las coordenadas Y para que coincidan con las dimensiones
 *          de la imagen inicial
 */
void DetectorCarasHailoSCRFD::extraeDetec( vector<DeteccionCaraHailo> *lstDet, string nomCapaCajas, string nomCapaConf, string nomCapaPuntosCara, float anchoImg, float alturaImg, float confMin, float escala[2], float factorEscalaXImg, float factorEscalaYImg  )
{    
    hailo_vstream_info_t infoCajas,infoConf,infoPuntos;
    vector<uint8_t>vecCajas,vecConf,vecPuntos;
    const uint8_t *lstConf;
    const uint8_t *lstCajas;
    const uint8_t *lstPuntos;
    int width, height,numAnclasCelda;
    float conf,valor,anchor_size;
    DeteccionCaraHailo det;

    infoCajas = runner.getOutVsTramInfo(nomCapaCajas);
    infoConf = runner.getOutVsTramInfo(nomCapaConf);    
    infoPuntos = runner.getOutVsTramInfo(nomCapaPuntosCara);    
    
    vecConf = runner.getOutputStream(nomCapaConf);
    lstConf = vecConf.data();

    vecCajas = runner.getOutputStream(nomCapaCajas);    
    lstCajas = vecCajas.data();

    vecPuntos = runner.getOutputStream(nomCapaPuntosCara);
    lstPuntos = vecPuntos.data();
    
    width = infoCajas.shape.width;
    height = infoCajas.shape.height;
    numAnclasCelda = 2;
    anchor_size = anchoImg / ((float)width);

    // int numTotalAnclas = width*height*numAnclasCelda;

    int i=0;
    
    for (int cell_y = 0; cell_y < height; cell_y++ )        
    {
        for (int cell_x = 0; cell_x < width; cell_x++ )        
        {
            for(int numAncla =0 ; numAncla < numAnclasCelda ; numAncla++ )
            {
                conf = runner.dequantize(lstConf[i], &infoConf.quant_info );
                // conf = runner.dequantize(lstConf[i], &infoCajas.quant_info );
                if ( conf < confMin )
                {
                    i++;   
                    continue;                
                }
                
                // Calcular centro de la grilla
                float x_center =  ((float)cell_x) * anchor_size / anchoImg;
                float y_center =  ((float)cell_y) * anchor_size / alturaImg;

                // calculo de factores de escala
                float factorEscalaX = escala[numAncla]  / anchoImg;
                float factorEscalaY = escala[numAncla]  / alturaImg;
                
                // Descuantiza los valores
                float dox1 = runner.dequantize(lstCajas[i * 4 + 0], &infoCajas.quant_info);
                float doy1 = runner.dequantize(lstCajas[i * 4 + 1], &infoCajas.quant_info);
                float dox2 = runner.dequantize(lstCajas[i * 4 + 2], &infoCajas.quant_info);
                float doy2 = runner.dequantize(lstCajas[i * 4 + 3], &infoCajas.quant_info);                

                float p1x = runner.dequantize(lstPuntos[i*10 + 0], &infoPuntos.quant_info);
                float p1y = runner.dequantize(lstPuntos[i*10 + 1], &infoPuntos.quant_info);
                float p2x = runner.dequantize(lstPuntos[i*10 + 2], &infoPuntos.quant_info);
                float p2y = runner.dequantize(lstPuntos[i*10 + 3], &infoPuntos.quant_info);
                float p3x = runner.dequantize(lstPuntos[i*10 + 4], &infoPuntos.quant_info);
                float p3y = runner.dequantize(lstPuntos[i*10 + 5], &infoPuntos.quant_info);
                float p4x = runner.dequantize(lstPuntos[i*10 + 6], &infoPuntos.quant_info);
                float p4y = runner.dequantize(lstPuntos[i*10 + 7], &infoPuntos.quant_info);
                float p5x = runner.dequantize(lstPuntos[i*10 + 8], &infoPuntos.quant_info);
                float p5y = runner.dequantize(lstPuntos[i*10 + 9], &infoPuntos.quant_info);

                // aplica factores de escala a los valores de coordenadas
                dox1*= factorEscalaX;
                doy1*= factorEscalaY;
                dox2*= factorEscalaX;
                doy2*= factorEscalaY;               

                p1x*= factorEscalaX;
                p1y*= factorEscalaY;
                p2x*= factorEscalaX;
                p2y*= factorEscalaY;
                p3x*= factorEscalaX;
                p3y*= factorEscalaY;
                p4x*= factorEscalaX;
                p4y*= factorEscalaY;
                p5x*= factorEscalaX;
                p5y*= factorEscalaY;                
                
                // Calcular coordenadas finales
                float x_min = ( x_center - dox1  ) * anchoImg ;  
                float y_min = ( y_center - doy1 ) * alturaImg ;
                float x_max = ( x_center + dox2  ) * anchoImg ;
                float y_max = ( y_center + doy2 ) * alturaImg ;                

                float pf1x = ( x_center + p1x  ) * anchoImg ;
                float pf1y = ( y_center + p1y  ) * alturaImg ;
                float pf2x = ( x_center + p2x  ) * anchoImg ;
                float pf2y = ( y_center + p2y  ) * alturaImg ;
                float pf3x = ( x_center + p3x  ) * anchoImg ;
                float pf3y = ( y_center + p3y  ) * alturaImg ;
                float pf4x = ( x_center + p4x  ) * anchoImg ;
                float pf4y = ( y_center + p4y  ) * alturaImg ;
                float pf5x = ( x_center + p5x  ) * anchoImg ;
                float pf5y = ( y_center + p5y  ) * alturaImg ;
                
                // Asegurar que las coordenadas estén dentro de la imagen
                det.ptoSupIzq.x = NumberUtils::getValorRango(x_min,0,anchoImg) * factorEscalaXImg;
                det.ptoSupIzq.y = NumberUtils::getValorRango(y_min,0,alturaImg) * factorEscalaYImg;
                det.ptoInfDer.x = NumberUtils::getValorRango(x_max,0,anchoImg) * factorEscalaXImg;
                det.ptoInfDer.y = NumberUtils::getValorRango(y_max,0,alturaImg) * factorEscalaYImg;

                det.ojoIzq.x = NumberUtils::getValorRango(pf1x,0,anchoImg) * factorEscalaXImg;
                det.ojoIzq.y = NumberUtils::getValorRango(pf1y,0,alturaImg) * factorEscalaYImg;
                det.ojoDer.x = NumberUtils::getValorRango(pf2x,0,anchoImg) * factorEscalaXImg;
                det.ojoDer.y = NumberUtils::getValorRango(pf2y,0,alturaImg) * factorEscalaYImg;
                det.nariz.x = NumberUtils::getValorRango(pf3x,0,anchoImg) * factorEscalaXImg;
                det.nariz.y = NumberUtils::getValorRango(pf3y,0,alturaImg) * factorEscalaYImg;
                det.bocaIzq.x = NumberUtils::getValorRango(pf4x,0,anchoImg) * factorEscalaXImg;
                det.bocaIzq.y = NumberUtils::getValorRango(pf4y,0,alturaImg) * factorEscalaYImg;
                det.bocaDer.x = NumberUtils::getValorRango(pf5x,0,anchoImg) * factorEscalaXImg;
                det.bocaDer.y = NumberUtils::getValorRango(pf5y,0,alturaImg) * factorEscalaYImg;
 
                det.confidencia = conf;
                        
                lstDet->push_back(det);            

                // incrementamos el lector
                i++;

            }  // fin bucle ancla

        } // fin bucle x            

    } // fin bucle y    
}


/**
 * Retorna las detecciones para el modelo 2.5g
 * 
 *      prec : 
 *          Porcentaje de precicion o accurary esperada
 */
std::vector<DeteccionCaraHailo> DetectorCarasHailoSCRFD::getDetecciones_2_5g( float prec )
{
    std::vector<DeteccionCaraHailo> lstDet, lstDetFinal;

    float escala1[2] = {8,8};
    float escala2[2] = {16,16};
    float escala3[2] = {32,32};

    float factorEscalaX;
    float factorEscalaY;   
    int deltaBorde;         

    if ( anchoImgOrig >= alturaImgOrig )
    {
        deltaBorde = (anchoImgOrig - alturaImgOrig)/2;
        factorEscalaX = anchoImgOrig / 640.0;
        factorEscalaY = factorEscalaX;
    }
    else
    {
        deltaBorde = (alturaImgOrig-anchoImgOrig)/2;
        factorEscalaY = alturaImgOrig / 640.0;            
        factorEscalaX = factorEscalaY;
    }
    
    extraeDetec(&lstDet,"scrfd_2_5g/conv43","scrfd_2_5g/conv42","scrfd_2_5g/conv44",640,640,prec, escala1, factorEscalaX, factorEscalaY);
    extraeDetec(&lstDet,"scrfd_2_5g/conv50","scrfd_2_5g/conv49","scrfd_2_5g/conv51",640,640,prec, escala2, factorEscalaX, factorEscalaY );
    extraeDetec(&lstDet,"scrfd_2_5g/conv56","scrfd_2_5g/conv55","scrfd_2_5g/conv57",640,640,prec, escala3, factorEscalaX, factorEscalaY );

    int n = lstDet.size();
    DeteccionCaraHailo det;

    for(int i=0; i<n; i++)
    {
        det = lstDet[i];        
        if ( anchoImgOrig >= alturaImgOrig )
        {   
            det.desplazaPtosCara(0,-deltaBorde);
            det.ptoSupIzq.y-= deltaBorde;
            det.ptoInfDer.y-= deltaBorde;
        }
        else
        {
            det.desplazaPtosCara(-deltaBorde,0);
            det.ptoSupIzq.x-= deltaBorde;
            det.ptoInfDer.x-= deltaBorde;

        }    
        lstDet[i] = det;
    }    

    lstDetFinal = apply_nms(lstDet, nms_iou_thresh);

    
    return lstDetFinal;
}

/**
 * Retorna las detecciones para el modelo 500m
 * 
 *      prec : 
 *          Porcentaje de precicion o accurary esperada
 */
std::vector<DeteccionCaraHailo> DetectorCarasHailoSCRFD::getDetecciones_500m( float prec )
{
    std::vector<DeteccionCaraHailo> lstDet;

    float escala1[2] = {8,8};
    float escala2[2] = {16,16};
    float escala3[2] = {32,32};

    float factorEscalaX;
    float factorEscalaY;
    int deltaBorde;            

    if ( anchoImgOrig >= alturaImgOrig )
    {
        deltaBorde = (anchoImgOrig - alturaImgOrig)/2;
        factorEscalaX = anchoImgOrig / 640.0;
        factorEscalaY = factorEscalaX;
    }
    else
    {
        deltaBorde = (alturaImgOrig-anchoImgOrig)/2;
        factorEscalaY = alturaImgOrig / 640.0;            
        factorEscalaX = factorEscalaY;
    }

    extraeDetec(&lstDet,"scrfd_500m/conv27","scrfd_500m/conv26", "scrfd_500m/conv25", 640,640,prec, escala1, factorEscalaX, factorEscalaY );
    extraeDetec(&lstDet,"scrfd_500m/conv33","scrfd_500m/conv32", "scrfd_500m/conv34", 640,640,prec, escala2, factorEscalaX, factorEscalaY );
    extraeDetec(&lstDet,"scrfd_500m/conv39","scrfd_500m/conv38","scrfd_500m/conv40", 640,640,prec, escala3, factorEscalaX, factorEscalaY );

    int n = lstDet.size();
    DeteccionCaraHailo det;

    for(int i=0; i<n; i++)
    {
        det = lstDet[i];        
        if ( anchoImgOrig >= alturaImgOrig )
        {   
            det.desplazaPtosCara(0,-deltaBorde);
            det.ptoSupIzq.y-= deltaBorde;
            det.ptoInfDer.y-= deltaBorde;
        }
        lstDet[i] = det;
    }   

    return apply_nms(lstDet, nms_iou_thresh);
}


/**
 * Retorna las detecciones para el modelo 500m
 * 
 *      prec : 
 *          Porcentaje de precicion o accurary esperada
 */
std::vector<DeteccionCaraHailo> DetectorCarasHailoSCRFD::getDetecciones_10g( float prec )
{
    std::vector<DeteccionCaraHailo> lstDet;

    float escala1[2] = {8,8};
    float escala2[2] = {16,16};
    float escala3[2] = {32,32};

    float factorEscalaX;
    float factorEscalaY;    
    int deltaBorde;        

    if ( anchoImgOrig >= alturaImgOrig )
    {
        deltaBorde = (anchoImgOrig - alturaImgOrig)/2;
        factorEscalaX = anchoImgOrig / 640.0;
        factorEscalaY = factorEscalaX;
    }
    else
    {
        deltaBorde = (alturaImgOrig-anchoImgOrig)/2;
        factorEscalaY = alturaImgOrig / 640.0;            
        factorEscalaX = factorEscalaY;
    }

    extraeDetec(&lstDet,"scrfd_10g/conv42","scrfd_10g/conv41", "scrfd_10g/conv43", 640,640,prec, escala1, factorEscalaX, factorEscalaY );
    extraeDetec(&lstDet,"scrfd_10g/conv50","scrfd_10g/conv49", "scrfd_10g/conv51", 640,640,prec, escala2, factorEscalaX, factorEscalaY );
    extraeDetec(&lstDet,"scrfd_10g/conv57","scrfd_10g/conv56","scrfd_10g/conv58", 640,640,prec, escala3, factorEscalaX, factorEscalaY );

    int n = lstDet.size();
    DeteccionCaraHailo det;

    for(int i=0; i<n; i++)
    {
        det = lstDet[i];        
        if ( anchoImgOrig >= alturaImgOrig )
        {   
            det.desplazaPtosCara(0,-deltaBorde);
            det.ptoSupIzq.y-= deltaBorde;
            det.ptoInfDer.y-= deltaBorde;
        }
        lstDet[i] = det;
    }   

    return apply_nms(lstDet, nms_iou_thresh);
}


/**
 * Ejecuta non max supression
 * Para eliminar detecciones repetidas
 */
vector<DeteccionCaraHailo> DetectorCarasHailoSCRFD::apply_nms(vector<DeteccionCaraHailo>& detections, float nms_iou_thresh) 
{
    if (detections.empty()) return {};
    
    sort(detections.begin(), detections.end(), [](const DeteccionCaraHailo& a, const DeteccionCaraHailo& b) {
        return a.confidencia > b.confidencia;
    });
    
    vector<DeteccionCaraHailo> final_detections;
    vector<bool> suppressed(detections.size(), false);
    
    for (size_t i = 0; i < detections.size(); ++i) {
        if (suppressed[i]) continue;
        final_detections.push_back(detections[i]);
        for (size_t j = i + 1; j < detections.size(); ++j) {
            float iou = compute_iou(detections[i], detections[j]);
            if (iou > nms_iou_thresh) suppressed[j] = true;
        }
    }
    return final_detections;
}

float DetectorCarasHailoSCRFD::compute_iou(const DeteccionCaraHailo& a, const DeteccionCaraHailo& b) 
{
    float inter_x1 = max(a.ptoSupIzq.x, b.ptoSupIzq.x);
    float inter_y1 = max(a.ptoSupIzq.y, b.ptoSupIzq.y);
    float inter_x2 = min(a.ptoInfDer.x, b.ptoInfDer.x);
    float inter_y2 = min(a.ptoInfDer.y, b.ptoInfDer.y);

    float inter_area = max(0.0f, inter_x2 - inter_x1) * max(0.0f, inter_y2 - inter_y1);
    float area_a = (a.ptoInfDer.x - a.ptoSupIzq.x) * (a.ptoInfDer.y - a.ptoSupIzq.y);
    float area_b = (b.ptoInfDer.x - b.ptoSupIzq.x) * (b.ptoInfDer.y - b.ptoSupIzq.y);

    return inter_area / (area_a + area_b - inter_area);
}


/**
 * Retorna las detecciones.
 * El analiza el tipo de modelo y llama al metodo de deteccion correcto
 * 
 *      prec : 
 *          Porcentaje de precicion o accurary esperada
 */
std::vector<DeteccionCaraHailo> DetectorCarasHailoSCRFD::detectar( GImage image, float prec )
{
    if ( tipoModelo == _DetectorCarasHailoSCRFD_500m_ ) return detectar_500m(image,prec);
    else
    if ( tipoModelo == _DetectorCarasHailoSCRFD_2_5g_ ) return detectar_2_5g(image,prec);
    else
    if ( tipoModelo == _DetectorCarasHailoSCRFD_10g_ ) return detectar_10g(image,prec);

    std::vector<DeteccionCaraHailo> blanco;

    return blanco;
}
       

/**
 * Retorna las detecciones
 * 
 *      image:
 *          Imagen desde la que se hacen las detecciones
 * 
 *      prec : 
 *          Porcentaje de precicion o accurary esperada
 */
std::vector<DeteccionCaraHailo> DetectorCarasHailoSCRFD::detectar_2_5g( GImage image, float prec )
{
    errorDeteccion = false;
    if ( ejecutar(image.imagenOpencv) == false )
    {
        vector<DeteccionCaraHailo> rpta;
        errorDeteccion = true;
        return rpta;
    }

    return getDetecciones_2_5g(prec);
}


/**
 * Retorna las detecciones
 * 
 *      image:
 *          Imagen desde la que se hacen las detecciones
 * 
 *      prec : 
 *          Porcentaje de precicion o accurary esperada
 */
std::vector<DeteccionCaraHailo> DetectorCarasHailoSCRFD::detectar_10g( GImage image, float prec )
{
    errorDeteccion = false;
    if ( ejecutar(image.imagenOpencv) == false )
    {
        vector<DeteccionCaraHailo> rpta;
        errorDeteccion = true;
        return rpta;
    }

    return getDetecciones_10g(prec);
}


/**
 * Retorna las detecciones
 * 
 *      image:
 *          Imagen desde la que se hacen las detecciones
 
 *      prec : 
 *          Porcentaje de precicion o accurary esperada
 */
std::vector<DeteccionCaraHailo> DetectorCarasHailoSCRFD::detectar_500m( GImage image, float prec )
{
    errorDeteccion = false;
    if ( ejecutar(image.imagenOpencv) == false )
    {
        vector<DeteccionCaraHailo> rpta;
        errorDeteccion = true;
        return rpta;
    }

    return getDetecciones_500m(prec);
}


