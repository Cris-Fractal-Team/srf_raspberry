

#ifndef _GRAPHICS_UTILS_
#define _GRAPHICS_UTILS_

#include <opencv2/opencv.hpp>

/**
 * Dada una imagen
 * Valida si esta o no a contraluz
 * 
 *  img: imagen que se analiza
 * 
 *  ratioOscuro : retorna un valor entre 0 y 1 indicando el porcentaje de pixeles oscuros
 * 
 *  ratioClaro : retorna un valor entre 0 y 1 indicando el porcentaje de pixles claros
 * 
 *  threshold_ratio : si la suma del ratio claro + ratio oscuro superan este valor se considera contraluz
 * 
 *  brightness_threshold : valor de los pixeles en blanco y negro que se consideran claros, todo los mayores a ese valor
 */
bool isBacklit( const cv::Mat &img, double *ratioOscuro, double *ratioClaro,  double threshold_ratio = 0.5, int brightness_threshold = 200, int darkness_threshold = 50 );


/**
 * Funcion que aplicac correcion gamma a una imagen
 * Si el valor gamma es mayor que uno incrementa el brillo, menor que uno lo decrementa
 */
cv::Mat applyGammaCorrection(const cv::Mat& img, double gamma);

#endif
