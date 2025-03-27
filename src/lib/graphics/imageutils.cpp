
#include "lib/graphics/imageutils.h"


bool isBacklit(const cv::Mat& img, double *ratioOscuro, double *ratioClaros, double threshold_ratio, int brightness_threshold, int darkness_threshold ) {
    // Convertir la imagen a escala de grises
    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    // Calcular el histograma de la imagen en escala de grises
    int histSize = 256; // Niveles de gris
    float range[] = {0, 256};
    const float* histRange = {range};
    cv::Mat hist;
    cv::calcHist(&gray, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

    // Calcular el porcentaje de pixeles brillantes y oscuros
    int totalPixels = gray.rows * gray.cols;
    int brightPixels = 0;
    int darkPixels = 0;

    for (int i = 0; i < histSize; ++i) {
        int count = static_cast<int>(hist.at<float>(i));

        // Pixeles oscuros (niveles de gris bajos)
        if (i < darkness_threshold) {
            darkPixels += count;
        }
        // Pixeles brillantes (niveles de gris altos)
        if (i > brightness_threshold) {
            brightPixels += count;
        }
    }

    double darkRatio = static_cast<double>(darkPixels) / totalPixels;
    double brightRatio = static_cast<double>(brightPixels) / totalPixels;

    // std::cout << "Ratio Oscuro " << darkRatio << " Ratio Brillo " << brightRatio << " Suma "  << ( darkRatio + brightRatio ) << " Limite " << threshold_ratio << std::endl;

    // Si el porcentaje de pixeles brillantes y oscuros supera un umbral, se considera contraluz
    // if (darkRatio > threshold_ratio && brightRatio > threshold_ratio) {
    if (( darkRatio + brightRatio ) > threshold_ratio )
    {    
        return true; // Imagen a contraluz
    }

    return false; // Imagen no esta a contraluz
}

/**
 * Funcion que aplicac correcion gamma a una imagen
 * Si el valor gamma es mayor que uno incrementa el brillo, menor que uno lo decrementa
 */
cv::Mat applyGammaCorrection(const cv::Mat& img, double gamma) 
{
    CV_Assert(gamma >= 0);  // Asegurarse de que el valor de gamma es valido

    cv::Mat img_gamma_corrected;
    cv::Mat lookUpTable(1, 256, CV_8U);  // Crear una tabla de busqueda para la correccion gamma

    // Llenar la tabla de busqueda con los valores gamma corregidos
    uchar* lut = lookUpTable.ptr();
    for (int i = 0; i < 256; i++) {
        lut[i] = cv::saturate_cast<uchar>(pow(i / 255.0, gamma) * 255.0);
    }

    // Aplicar la correccion gamma usando la tabla de b�squeda
    cv::LUT(img, lookUpTable, img_gamma_corrected);

    return img_gamma_corrected;
}