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

VipsImage* invert(VipsImage* img){

    VipsImage *scInverted;

    if( vips_invert(img, &scInverted, NULL ) )
        vips_error_exit( NULL );

    return scInverted; 
}

VipsImage* gaussianblur(VipsImage* img){

    VipsImage *gauss;

    if( vips_gaussblur( img, &gauss, 15.00, NULL ) )
        vips_error_exit( NULL );

    return gauss;
}

VipsImage* canny(VipsImage* img){

    VipsImage *scCanny;

    if( vips_gaussblur( img, &scCanny, 15.00, NULL) )
        vips_error_exit( NULL );

    return scCanny;

        
}

VipsImage* sobel(VipsImage* img){

    VipsImage *scSobel;

    if( vips_gaussblur( img, &scSobel, 15.00, NULL) )
        vips_error_exit( NULL );

    return scSobel;

}

int main( int argc, char **argv )
{
    VipsImage *in;
    //double mean;
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


    VipsImage* inverted = invert(in);

    if( vips_image_write_to_file(inverted, "inverted.jpg", NULL ) )
        vips_error_exit( NULL );


    VipsImage* gaussblur = gaussianblur(in);

    if( vips_image_write_to_file(gaussblur, "gaussblur.jpg", NULL ) )
        vips_error_exit( NULL );


    VipsImage* cannypic = canny(in);

    if( vips_image_write_to_file(cannypic, "canny.jpg", NULL ) )
        vips_error_exit( NULL );


    VipsImage* sobelpic = sobel(in);

    if( vips_image_write_to_file(sobelpic, "inverted.jpg", NULL ) )
        vips_error_exit( NULL );

    
    
    g_object_unref(in); 
/*
    if( vips_image_write_to_file( out, argv[2], NULL ) )
        vips_error_exit( NULL );

    g_object_unref( out ); */
    g_object_unref(gray);
    g_object_unref(inverted);
    g_object_unref(gaussblur);
    g_object_unref(cannypic);
    g_object_unref(sobelpic);

    return( 0 );
}
