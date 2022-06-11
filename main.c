#include <stdio.h>
#include <vips/vips.h>


VipsImage* grayscale(VipsImage* img){

    VipsImage *scRGB;

    if(vips_sRGB2scRGB(img, &scRGB, NULL))
        vips_error_exit(NULL);

    VipsImage *gray;
    if( vips_scRGB2BW(scRGB, &gray, NULL))
        vips_error_exit( NULL );

    return gray;
}

VipsImage* hsvTrans(VipsImage* img){

    VipsImage *scRGB;

    if(vips_sRGB2scRGB(img, &scRGB, NULL))
        vips_error_exit(NULL);


    VipsImage *sRGB;
    if(vips_scRGB2sRGB(scRGB,&sRGB, NULL))
        vips_error_exit(NULL);

    VipsImage* hsv;
    if(vips_sRGB2HSV(sRGB, &hsv, NULL))
        vips_error_exit(NULL);
    return hsv; 
}

// VipsImage* gaussianblur(VipsImage* img){

// }
    // VipsImage* canny(VipsIMage* img){
        
    // }



/*
VipsImage* sobel(VipsImage* img){

}
*/

int main( int argc, char **argv )
{
    VipsImage *in;
    double mean;
    //VipsImage *out;

    if( VIPS_INIT( argv[0] ) )
        vips_error_exit( NULL ); 

    if( argc != 3 )
        vips_error_exit( "usage: %s infile outfile", argv[0] ); 

    if( !(in = vips_image_new_from_file( argv[1], NULL )) )
        vips_error_exit( NULL );
/*
    printf( "image width = %d\n", vips_image_get_width( in ) ); 

    if( vips_avg( in, &mean, NULL ) )
        vips_error_exit( NULL );

    printf( "mean pixel value = %g\n", mean ); 

    if( vips_invert( in, &out, NULL ) )
        vips_error_exit( NULL );
*/
    VipsImage* gray = grayscale(in);


    if( vips_image_write_to_file( gray, "gray.png", NULL ) )
        vips_error_exit( NULL );


    VipsImage* hsv = hsvTrans(in);

    if( vips_image_write_to_file( hsv, "hsv.jpg", NULL ) )
        vips_error_exit( NULL );


    
    
    g_object_unref( in ); 
/*
    if( vips_image_write_to_file( out, argv[2], NULL ) )
        vips_error_exit( NULL );

    g_object_unref( out ); */
    g_object_unref(gray);
    g_object_unref(hsv);

    return( 0 );
}