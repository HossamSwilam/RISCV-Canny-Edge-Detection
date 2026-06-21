// --- حفظ الصور بعد كل مرحلة ---
    std::cout << "Saving intermediate images..." << std::endl;
    writeRaw("0_input.raw", input);
    writeRaw("1_blurred.raw", blurred);
    writeRaw("2_magnitude.raw", magnitude);
    
    // صورة الاتجاهات (Direction) بتكون أرقامها صغيرة (0, 45, 90, 135) 
    // عشان تشوفها بعينك لازم نضربها في رقم عشان تنور في الصورة
    Image vis_direction;
    vis_direction.allocate(width, height);
    for(int i=0; i<width*height; i++) {
        vis_direction.data[i] = direction.data[i] * 50; // تكبير القيم للرؤية
    }
    writeRaw("3_direction.raw", vis_direction);
    vis_direction.free_memory();
