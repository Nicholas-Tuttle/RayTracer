#include <iostream>
#include <chrono>

#include <Camera.h>
#include <IImage.h>
#include <IScene.h>
#include <CPURenderer.h>
#include <GPURenderer.h>
#include <GPURenderer.h>
#include <Scene.h>
#include <Sphere.h>
#include <Utilities.h>

#include <PresetScenes.h>
#include "TinyOBJLoader.h"

using RayTracer::IScene;
using RayTracer::Camera;
using RayTracer::CPURenderer;
using RayTracer::GPURenderer;
using RayTracer::GPURenderer;
using RayTracer::ImageResolution;
using RayTracer::IImage;
using RayTracer::Color;

class InputParser 
{
public:
    InputParser(int argc, char **argv) 
    {
        arguments.reserve((size_t)argc - 1);
        for (int i = 1; i < argc; ++i)
        {
            arguments.push_back(std::string(argv[i]));
        }
    }

    const std::string GetCommandOption(const std::string &option) const 
    {
        auto itr = std::find(arguments.begin(), arguments.end(), option);
        if (itr != arguments.end() && ++itr != arguments.end()) 
        {
            return *itr;
        }
        return std::string();
    }

    bool CommandOptionExists(const std::string &option) const 
    {
        return std::find(arguments.begin(), arguments.end(), option) != arguments.end();
    }
private:
    std::vector <std::string> arguments;
};

struct CommandLineArguments
{
    // Default to 1 sample on the GPU
    CommandLineArguments()
    {
        RenderCPU = false;
        RenderGPU = true;
        GPUDebugEnabled = false;
        TracePerformance = false;
        Samples = 1;
        MaxBounces = 4;
        ResolutionX = 1920;
        ResolutionY = 1080;
        MaxThreads = 0;
        ShowHelp = false;
        output_file_path = "";
        input_file_path = "";
    }

    bool RenderCPU;
    bool RenderGPU;
    bool GPUDebugEnabled;
    bool TracePerformance;
    unsigned int Samples;
    size_t MaxBounces;
    size_t ResolutionX;
    size_t ResolutionY;
    size_t MaxThreads;
    bool ShowHelp;
    std::string output_file_path;
    std::string input_file_path;
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

    const std::string max_bounces_string = parser.GetCommandOption("-b");
    if (0 != max_bounces_string.compare(""))
    {
        size_t max_bounces = (size_t)std::stoul(max_bounces_string);
        arguments.MaxBounces = max_bounces;
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

    const std::string output_file_path_string = parser.GetCommandOption("-o");
    if (0 != output_file_path_string.compare(""))
    {
        arguments.output_file_path = output_file_path_string;
    }

    const std::string input_file_path_string = parser.GetCommandOption("-i");
    if (0 != input_file_path_string.compare(""))
    {
        arguments.input_file_path = input_file_path_string;
    }

    bool render_cpu = parser.CommandOptionExists("-c");
    arguments.RenderCPU = render_cpu;

    bool render_gpu = parser.CommandOptionExists("-g");
    arguments.RenderGPU = render_gpu;

    bool gpu_debug = parser.CommandOptionExists("-dg");
    arguments.GPUDebugEnabled = gpu_debug;

    bool trace_performance = parser.CommandOptionExists("-t");
    arguments.TracePerformance = trace_performance;

    bool show_help = parser.CommandOptionExists("-h");
    arguments.ShowHelp = show_help;
}

static void help()
{
    std::cout << "RayTracer.exe\n"
        << "\tArguments:\n"
        << "\t\t-h : show help\n"
        << "\t\t-c : render on CPU\n"
        << "\t\t-g : render on GPU\n"
        << "\t\t-dg : enable GPU debug messages\n"
        << "\t\t-t : enable performance tracing\n"
        << "\t\t-o <path> : output file path\n"
        << "\t\t-i <path> : input file path\n"
        << "\t\t-s <samples> : set sample count [ default 1 ]\n"
        << "\t\t-b <bounces> : set max bounces [ default 4 ]\n"
        << "\t\t-x <x resolution> : set image width (pixels) [ default 1920 ]\n"
        << "\t\t-y <y resolution> : set image height (pixels) [ default 1080 ]\n"
        << "\t\t-m <max threads> : set the max number of threads the cpu renderer can use [ default inf ]\n";
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

int main(int argc, char **argv)
{
    // Determine sample count from command line if possible
    CommandLineArguments arguments;
    parse_command_line_arguments(argc, argv, arguments);

    if (arguments.ShowHelp || arguments.output_file_path == "")
    {
        help();
        exit(0);
    }

    std::cout << "Rendering with " << arguments.Samples << " samples" << std::endl;

    ImageResolution resolution(arguments.ResolutionX, arguments.ResolutionY);

    // Build a scene to render
    IScene *scene = nullptr;
    Camera *camera = nullptr;

    if (0 == arguments.input_file_path.compare(""))
    {
        // Create a sample scene if things are empty
        RayTracer::CreatePresetScene1(scene, camera, resolution);
    }
    else
    {
        RayTracer::CreateSceneFromOBJFile(arguments.input_file_path, resolution, camera, scene);
    }

    // Provide an output pointer for the image
    std::shared_ptr<IImage> out_image = nullptr;
    auto tm0 = std::chrono::high_resolution_clock::now();

    if (arguments.RenderCPU)
    {
        std::cout << std::endl << "Rendering on CPU" << std::endl;
        CPURenderer cpu_renderer;
        cpu_renderer.Render(arguments.MaxThreads, *camera, arguments.Samples, *scene, out_image);
    }

    if (arguments.RenderGPU)
    {
        std::cout << std::endl << "Rendering on GPU" << std::endl;
        GPURenderer::GPURendererInitParameters init_params(*camera, *scene);
        init_params.samples = arguments.Samples;
        init_params.max_bounces = arguments.MaxBounces;
        init_params.trace_performance = arguments.TracePerformance;
        GPURenderer gpu_renderer(init_params);
        gpu_renderer.Render(out_image);
    }

    auto tm1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> total_time = tm1 - tm0;
    std::cout << "Total Render Time (ms): " << total_time.count() << std::endl;

    if (nullptr != out_image)
    {
        write_png_file(arguments.output_file_path, out_image->GetColorRGBAValues());
    }
    else
    {
        std::cout << "Failed to create output image" << std::endl;
    }

    std::cout << std::endl;

    return 0;
}

