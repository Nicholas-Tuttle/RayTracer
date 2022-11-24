#include <iostream>
#include <chrono>

#include <Camera.h>
#include <IImage.h>
#include <IScene.h>
#include <CPURenderer.h>
#include <GPURenderer.h>
#include <GPURendererV2.h>
#include <Scene.h>
#include <Sphere.h>
#include <Material.h>
#include <Utilities.h>

#include <PresetScenes.h>

using RayTracer::IScene;
using RayTracer::Camera;
using RayTracer::CPURenderer;
using RayTracer::GPURenderer;
using RayTracer::GPURendererV2;
using RayTracer::ImageResolution;
using RayTracer::IImage;
using RayTracer::Color;

class InputParser 
{
public:
    InputParser(int& argc, char** argv) 
    {
        tokens.reserve((size_t)argc - 1);
        for (int i = 1; i < argc; ++i)
        {
            tokens.push_back(std::string(argv[i]));
        }
    }

    const std::string& GetCommandOption(const std::string& option) const 
    {
        std::vector<std::string>::const_iterator itr = std::find(tokens.begin(), tokens.end(), option);
        if (itr != tokens.end() && ++itr != tokens.end()) 
        {
            return *itr;
        }
        return std::string();
    }

    bool CommandOptionExists(const std::string& option) const 
    {
        return std::find(tokens.begin(), tokens.end(), option) != tokens.end();
    }
private:
    std::vector <std::string> tokens;
};

struct CommandLineArguments
{
    // Default to 1 sample on the GPU
    CommandLineArguments()
    {
        RenderCPU = false;
        RenderGPU = true;
        GPUDebugEnabled = false;
        Samples = 1;
        ResolutionX = 1920;
        ResolutionY = 1080;
        MaxThreads = 0;
        ShowHelp = false;
    }

    bool RenderCPU;
    bool RenderGPU;
    bool GPUDebugEnabled;
    unsigned int Samples;
    size_t ResolutionX;
    size_t ResolutionY;
    size_t MaxThreads;
    bool ShowHelp;
};

static void parse_command_line_arguments(int argc, char** argv, CommandLineArguments& arguments)
{
    InputParser parser(argc, argv);

    const std::string samples_string = parser.GetCommandOption("-s");
    if (0 != samples_string.compare(""))
    {
        unsigned int samples = (unsigned int)std::stoul(samples_string);
        arguments.Samples = samples;
    }
    
    const std::string resolution_x_string = parser.GetCommandOption("-x");
    if (0 != resolution_x_string.compare(""))
    {
        size_t resolution_x = (size_t)std::stoul(resolution_x_string);
        arguments.ResolutionX = resolution_x;
    }

    const std::string resolution_y_string = parser.GetCommandOption("-y");
    if (0 != resolution_y_string.compare(""))
    {
        size_t resolution_y = (size_t)std::stoul(resolution_y_string);
        arguments.ResolutionY = resolution_y;
    }

    const std::string max_threads_string = parser.GetCommandOption("-m");
    if (0 != max_threads_string.compare(""))
    {
        size_t max_threads = (size_t)std::stoul(max_threads_string);
        arguments.MaxThreads = max_threads;
    }

    bool render_cpu = parser.CommandOptionExists("-c");
    arguments.RenderCPU = render_cpu;

    bool render_gpu = parser.CommandOptionExists("-g");
    arguments.RenderGPU = render_gpu;

    bool gpu_debug = parser.CommandOptionExists("-dg");
    arguments.GPUDebugEnabled = gpu_debug;

    bool show_help = parser.CommandOptionExists("-h");
    arguments.ShowHelp = show_help;
}

static void help()
{
    std::cout << "RayTracer.exe" << std::endl
        << "\tArguments:" << std::endl
        << "\t\t-h : show help" << std::endl
        << "\t\t-c : render on CPU" << std::endl
        << "\t\t-g : render on GPU" << std::endl
        << "\t\t-dg : enable GPU debug messages" << std::endl
        << "\t\t-s <samples> : set sample count [ default 1 ]" << std::endl
        << "\t\t-x <x resolution> : set image width (pixels) [ default 1920 ]" << std::endl
        << "\t\t-y <y resolution> : set image height (pixels) [ default 1080 ]" << std::endl
        << "\t\t-m <max threads> : set the max number of threads the cpu renderer can use [ default inf ]" << std::endl;
}

static bool write_png_file(const std::string &file_name, const std::vector<std::vector<png_byte>> &color_values)
{
    FILE *file = nullptr;
    if (0 != fopen_s(&file, file_name.c_str(), "wb") || nullptr == file)
    {
        return false;
    }

    png_structp png_pointer = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (nullptr == png_pointer)
    {
        return false;
    }

    png_infop png_info_pointer = png_create_info_struct(png_pointer);
    if (nullptr == png_info_pointer)
    {
        return false;
    }

    if (setjmp(png_jmpbuf(png_pointer)))
    {
        return false;
    }

    png_init_io(png_pointer, file);

    png_uint_32 height = (png_uint_32)color_values.size();
    if (0 == height)
    {
        return false;
    }

    png_uint_32 width = (png_uint_32)color_values[0].size();
    if (width == 0 || (width % 4) != 0)
    {
        return false;
    }
    png_uint_32 png_pixel_width = (png_uint_32)(width / 4);

    png_byte bit_depth = 8; //TODO: parameterize
    png_byte color_type = PNG_COLOR_TYPE_RGBA; //TODO: parameterize

    /*
        width		- holds the width of the image in pixels (up to 2^31).

        height		- holds the height of the image in pixels (up to 2^31).

        bit_depth	- holds the bit depth of one of the image channels. (valid values are 1, 2, 4, 8, 16 and depend also on the color_type.  See also significant bits (sBIT) below).

        color_type     - describes which color/alpha
                         channels are present.
                         PNG_COLOR_TYPE_GRAY
                            (bit depths 1, 2, 4, 8, 16)
                         PNG_COLOR_TYPE_GRAY_ALPHA
                            (bit depths 8, 16)
                         PNG_COLOR_TYPE_PALETTE
                            (bit depths 1, 2, 4, 8)
                         PNG_COLOR_TYPE_RGB
                            (bit_depths 8, 16)
                         PNG_COLOR_TYPE_RGB_ALPHA
                            (bit_depths 8, 16)

                         PNG_COLOR_MASK_PALETTE
                         PNG_COLOR_MASK_COLOR
                         PNG_COLOR_MASK_ALPHA

        interlace_type - PNG_INTERLACE_NONE or
                         PNG_INTERLACE_ADAM7

        compression_type - (must be
                         PNG_COMPRESSION_TYPE_DEFAULT)

        filter_method  - (must be PNG_FILTER_TYPE_DEFAULT
                         or, if you are writing a PNG to
                         be embedded in a MNG datastream,
                         can also be
                         PNG_INTRAPIXEL_DIFFERENCING)
    */

    png_set_IHDR(png_pointer, png_info_pointer, png_pixel_width, height, bit_depth, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_pointer, png_info_pointer);

    if (setjmp(png_jmpbuf(png_pointer)))
    {
        return false;
    }

    png_bytepp row_pointers = (png_bytep *)calloc(height, sizeof(png_bytep));
    if (nullptr == row_pointers)
    {
        return false;
    }

    for (png_uint_32 i = 0; i < height; i++)
    {
        if (width != color_values[i].size())
        {
            return false;
        }

        row_pointers[i] = (png_byte *)calloc(1, png_get_rowbytes(png_pointer, png_info_pointer));
        if (nullptr == row_pointers[i])
        {
            return false;
        }

        for (png_uint_32 j = 0; j < width; j++)
        {
            row_pointers[i][j] = color_values[i][j];
        }
    }

    png_write_image(png_pointer, row_pointers);

    for (png_uint_32 i = 0; i < height; i++)
    {
        free(row_pointers[i]);
    }
    free(row_pointers);

    if (setjmp(png_jmpbuf(png_pointer)))
    {
        return false;
    }

    png_write_end(png_pointer, NULL);

    fclose(file);

    return true;
}

void run(const CommandLineArguments arguments)
{
    std::cout << "Rendering with " << arguments.Samples << " samples" << std::endl;

    ImageResolution resolution(arguments.ResolutionX, arguments.ResolutionY);

    // Build a scene to render
    IScene *scene = nullptr;
    std::shared_ptr<Camera> camera = nullptr;

    CreatePresetScene1Sphere1Light(scene, camera, resolution);

    if (arguments.RenderCPU)
    {
        std::cout << std::endl << "Rendering on CPU" << std::endl;
        CPURenderer cpu_renderer;

        // Provide an output pointer for the CPU image
        std::shared_ptr<IImage> out_cpu_image = nullptr;

        // Render on the CPU
        auto tm0 = std::chrono::high_resolution_clock::now();
        cpu_renderer.Render(arguments.MaxThreads, *camera, arguments.Samples, *scene, out_cpu_image);
        auto tm1 = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> cpuTime = tm1 - tm0;
        std::cout << "Total CPU Time (ms): " << cpuTime.count() << std::endl;

        write_png_file("..\\demo_cpu.png", out_cpu_image->GetColorRGBAValues());
    }

    if (arguments.RenderGPU)
    {
        /*
        std::cout << std::endl << "Rendering on GPU" << std::endl;
        GPURenderer gpu_renderer;
        gpu_renderer.GPUDebugEnabled = arguments.GPUDebugEnabled;
        
        // Provide an output pointer for the GPU image
        std::shared_ptr<IImage> out_gpu_image = nullptr;
        
        // Render on the GPU
        auto tm2 = std::chrono::high_resolution_clock::now();
        gpu_renderer.Render(*camera, arguments.Samples, scene, out_gpu_image);
        auto tm3 = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> gpuTime = tm3 - tm2;
        std::cout << "Total GPU Time (ms): " << gpuTime.count() << std::endl;
        
        write_png_file("..\\demo_gpu.png", out_gpu_image->GetColorRGBAValues());
        */

        std::cout << std::endl << "Rendering on GPU V2" << std::endl;
        GPURendererV2 gpu_renderer_v2(*camera);
        std::shared_ptr<IImage> out_dont_care = nullptr;
        gpu_renderer_v2.Render(0, *scene, out_dont_care);
    }


    std::cout << std::endl;
}

#if _WIN32

#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) 
{
    // Determine sample count from command line if possible
    CommandLineArguments arguments;
    parse_command_line_arguments(__argc, __argv, arguments);

    if (arguments.ShowHelp)
    {
        help();
        exit(0);
    }

    run(arguments);

    return 0;
}

#endif