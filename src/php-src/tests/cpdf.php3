<?php
  function draw_text($cpdf, $fontname, $size, $x, $y, $text="") {
    cpdf_begin_text($cpdf);
    cpdf_set_font($cpdf, $fontname, $size, 4);
    if($text == "")
      cpdf_text($cpdf, $fontname." ".$size." Point", $x, $y, 1);
    else
      cpdf_text($cpdf, $text, $x, $y, 1);
    cpdf_end_text($cpdf);
  }

  $cpdf = cpdf_open(0);
  cpdf_set_Creator($cpdf, "Creator");
  cpdf_set_Title($cpdf, "Title");
  cpdf_set_Subject($cpdf, "Subject");
  cpdf_set_Keywords($cpdf, "Keyword1, Keyword2");
  cpdf_set_viewer_preferences($cpdf, 1);

  cpdf_page_init($cpdf, 1, 0, 600, 400, 1);
  $level1 = cpdf_add_outline($cpdf, 0, 0, 1, 1, "Examples");
  $level2a = cpdf_add_outline($cpdf, $level1, 1, 0, 1, "Text");
  draw_text($cpdf, "Helvetica", 18.0, 50, 330);
  draw_text($cpdf, "Helvetica-Bold", 18.0, 50, 310);
  draw_text($cpdf, "Helvetica-Oblique", 18.0, 50, 290);
  draw_text($cpdf, "Helvetica-BoldOblique", 18.0, 50, 270);
  draw_text($cpdf, "Times-Roman", 18.0, 50, 250);
  draw_text($cpdf, "Times-Bold", 18.0, 50, 230);
  draw_text($cpdf, "Times-Italic", 18.0, 50, 210);
  draw_text($cpdf, "Times-BoldItalic", 18.0, 50, 190);
  draw_text($cpdf, "Courier", 18.0, 50, 170);
  draw_text($cpdf, "Courier-Bold", 18.0, 50, 150);
  draw_text($cpdf, "Courier-Oblique", 18.0, 50, 130);
  draw_text($cpdf, "Courier-BoldOblique", 18.0, 50, 110);
  draw_text($cpdf, "Symbol", 18.0, 50, 90);
  draw_text($cpdf, "ZapfDingbats", 18.0, 50, 70);

  cpdf_page_init($cpdf, 2, 0, 400, 400, 1);
  $level2b = cpdf_add_outline($cpdf, $level2a, 0, 0, 2, "Annotations, Hyperlinks");
  cpdf_set_annotation($cpdf, 100, 100, 300, 200, "Annotation", "Text of Annotation");
  draw_text($cpdf, "Helvetica", 12.0, 102, 116, "Click here to go to");
  draw_text($cpdf, "Helvetica", 12.0, 102, 103, "http://localhost");
  cpdf_set_action_url($cpdf, 100, 100, 202, 130, "http://localhost");

  cpdf_page_init($cpdf, 3, 0, 400, 600, 1);
  $level2c = cpdf_add_outline($cpdf, $level2b, 0, 0, 3, "Drawing");
  cpdf_rect($cpdf, 100, 100, 90, 90, 1);
  cpdf_stroke($cpdf);
  cpdf_rect($cpdf, 200, 100, 90, 90, 1);
  cpdf_fill_stroke($cpdf);

  cpdf_setgray_fill($cpdf, 0.5);
  cpdf_circle($cpdf, 140, 250, 35, 1);
  cpdf_stroke($cpdf);
  cpdf_circle($cpdf, 240, 250, 35, 1);
  cpdf_fill_stroke($cpdf);

  cpdf_moveto($cpdf, 300, 100, 1);
  cpdf_lineto($cpdf, 350, 100, 1);
  cpdf_curveto($cpdf, 350, 100, 325, 300, 300, 100, 1);
  cpdf_stroke($cpdf);
  
  cpdf_moveto($cpdf, 400, 100, 1);
  cpdf_lineto($cpdf, 450, 100, 1);
  cpdf_curveto($cpdf, 450, 100, 425, 400, 400, 100, 1);
  cpdf_fill_stroke($cpdf);
  
  cpdf_page_init($cpdf, 4, 0, 400, 600, 1);
  $level2d = cpdf_add_outline($cpdf, $level2c, 0, 0, 4, "Images, Clipping");
  cpdf_import_jpeg($cpdf, "figure.jpg", 50, 50, -10.0, 0.0, 300.0, 0.0, 0.0, 1, 1);
  cpdf_save($cpdf);
  cpdf_newpath($cpdf);
  cpdf_moveto($cpdf, 300, 50);
  cpdf_lineto($cpdf, 450, 50, 1);
  cpdf_curveto($cpdf, 450, 100, 375, 300, 300, 100, 1);
  cpdf_closepath($cpdf);
  cpdf_rect($cpdf, 350, 215, 100, 30);
  cpdf_clip($cpdf);
  cpdf_newpath($cpdf);  /* needed because clip doesn't consume path */
  draw_text($cpdf, "Helvetica", 18.0, 250, 130);
  cpdf_import_jpeg($cpdf, "figure.jpg", 300, 50, 0.0, 0.0, 300.0, 0.0, 0.0, 0, 1);  /* watch for the second last parameter. It is 0 to disable extra gsave/grestore */

  cpdf_restore($cpdf);

  cpdf_finalize($cpdf);
  Header("Content-type: application/pdf");
  cpdf_output_buffer($cpdf);
  cpdf_close($cpdf);
?>
