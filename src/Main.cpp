#include <iostream>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

using namespace std;

bool menuAberto = false;
int menuSelecao = -1;
int menuBotaoPressionado = -1;
bool modoDesenho = false;
int corDesenhoAtual = 1;

int ultimoSelecaoX = 0;
int ultimoSelecaoY = 0;
int ultimoSelecaoZ = 0;

int count = 0;

const GLuint WIDTH = 800, HEIGHT = 600;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 100.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = 0.0f;
bool firstMouse = true;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
float fov = 45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

GLuint shaderID, VAO;
GLFWwindow *window;

struct Voxel
{
    glm::vec3 pos;
    float fatorEscala;
    bool visivel = true, selecionado = false;
    int corPos;
};

int selecaoX, selecaoY, selecaoZ;
int TAM;
Voxel ***grid = nullptr;

glm::vec4 colorList[] = {
    {0.5f, 0.5f, 0.5f, 0.5f},
    {1.0f, 0.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 1.0f, 1.0f}
};

const GLchar *vertexShaderSource = R"glsl(
#version 450
layout(location = 0) in vec3 position;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
void main() {
    gl_Position = proj * view * model * vec4(position, 1.0);
}
)glsl";

const GLchar *fragmentShaderSource = R"glsl(
#version 450
uniform vec4 uColor;
out vec4 color;
void main() {
    color = uColor;
}
)glsl";

void salvarGradeVoxel(const std::string &nomeArquivo)
{
    std::ofstream arquivo(nomeArquivo);
    if (!arquivo.is_open())
    {
        std::cerr << "Erro ao abrir arquivo para escrita.\n";
        return;
    }
    arquivo << TAM << "\n";
    for (int x = 0; x < TAM; ++x)
    {
        for (int y = 0; y < TAM; ++y)
        {
            for (int z = 0; z < TAM; ++z)
            {
                const Voxel &v = grid[x][y][z];
                arquivo << v.pos.x << " " << v.pos.y << " " << v.pos.z << " "
                        << v.fatorEscala << " "
                        << v.visivel << " " << v.selecionado << " "
                        << v.corPos << "\n";
            }
        }
    }
    arquivo.close();
    cout << "Jogo salvo em " << nomeArquivo << endl;
}

void carregarGradeVoxel(const std::string &nomeArquivo)
{
    std::ifstream arquivo(nomeArquivo);
    if (!arquivo.is_open())
    {
        std::cerr << "Erro ao abrir arquivo para leitura.\n";
        return;
    }
    arquivo >> TAM;
    if (grid == nullptr) {
        grid = new Voxel **[TAM];
        for (int x = 0; x < TAM; ++x) {
            grid[x] = new Voxel *[TAM];
            for (int y = 0; y < TAM; ++y) {
                grid[x][y] = new Voxel[TAM];
            }
        }
    }
    for (int x = 0; x < TAM; ++x)
    {
        for (int y = 0; y < TAM; ++y)
        {
            for (int z = 0; z < TAM; ++z)
            {
                Voxel &v = grid[x][y][z];
                arquivo >> v.pos.x >> v.pos.y >> v.pos.z >> v.fatorEscala >> v.visivel >> v.selecionado >> v.corPos;
            }
        }
    }
    arquivo.close();
    cout << "Jogo carregado de " << nomeArquivo << endl;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (menuAberto || modoDesenho) return;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);

    glm::vec3 right = glm::normalize(glm::cross(cameraFront, glm::vec3(0.0, 1.0, 0.0)));
    cameraUp = glm::normalize(glm::cross(right, cameraFront));
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (menuAberto) return;

    if (fov >= 1.0f && fov <= 120.0f) fov -= yoffset;
    if (fov <= 1.0f) fov = 1.0f;
    if (fov >= 120.0f) fov = 120.0f;
}

void ativarOpcaoMenu() {
    if (menuSelecao == -1) return;
    switch (menuSelecao) {
        case 0:
            salvarGradeVoxel("minecraft.txt");
            break;
        case 1:
            carregarGradeVoxel("minecraft.txt");
            break;
        case 2:
            glfwSetWindowShouldClose(window, true);
            break;
    }
}

void inicializarGradeVoxel(int tamanho);
void reiniciarGradeVoxel(int novoTamanho);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        modoDesenho = false;
        menuAberto = !menuAberto;
        if (menuAberto)
        {
            menuSelecao = 0;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
    }
    
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
    {
        modoDesenho = !modoDesenho;
        if (modoDesenho) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            if (selecaoX != -1) {
                ultimoSelecaoX = selecaoX;
                ultimoSelecaoY = selecaoY;
                ultimoSelecaoZ = selecaoZ;
                grid[ultimoSelecaoY][ultimoSelecaoX][ultimoSelecaoZ].selecionado = false;
            }
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
            if (selecaoX != -1) {
                grid[selecaoY][selecaoX][selecaoZ].selecionado = false;
            }
            selecaoX = ultimoSelecaoX;
            selecaoY = ultimoSelecaoY;
            selecaoZ = ultimoSelecaoZ;
            if (selecaoX != -1) {
                 grid[selecaoY][selecaoX][selecaoZ].selecionado = true;
            }
        }
    }

    if (modoDesenho && !menuAberto && action == GLFW_PRESS) {
        if (key >= GLFW_KEY_1 && key <= GLFW_KEY_6) {
            corDesenhoAtual = key - GLFW_KEY_0;
            cout << "Cor de desenho alterada para: " << corDesenhoAtual << endl;
        }
    }
    
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
    {
        int novoTamanho;
        if (TAM == 32) {
            novoTamanho = 16;
        }
        else if (TAM == 16) {
            novoTamanho = 101;
        }
        else {
            novoTamanho = 32;
        }
        reiniciarGradeVoxel(novoTamanho);
    }

    if (menuAberto)
    {
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_W) {
                if(menuSelecao == -1) menuSelecao = 2;
                else menuSelecao--;
                if (menuSelecao < 0) menuSelecao = 2;
            }
            else if (key == GLFW_KEY_S) {
                if(menuSelecao == -1) menuSelecao = 0;
                else menuSelecao++;
                if (menuSelecao > 2) menuSelecao = 0;
            }
            else if (key == GLFW_KEY_SPACE) {
                ativarOpcaoMenu();
            }
        }
        return; 
    }
    
    if (modoDesenho) return;
    
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS){salvarGradeVoxel("minecraft.txt");}
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS){carregarGradeVoxel("minecraft.txt");}
    if (key == GLFW_KEY_DELETE && action == GLFW_PRESS){if(selecaoX != -1) grid[selecaoY][selecaoX][selecaoZ].visivel = false;}
    if (key == GLFW_KEY_V && action == GLFW_PRESS){if(selecaoX != -1) grid[selecaoY][selecaoX][selecaoZ].visivel = true;}
    bool mudouSelecao = false;
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){if (selecaoX + 1 < TAM){if(selecaoX!=-1)grid[selecaoY][selecaoX][selecaoZ].selecionado = false;selecaoX++;mudouSelecao = true;grid[selecaoY][selecaoX][selecaoZ].selecionado = true;}}
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){if (selecaoX - 1 >= 0){if(selecaoX!=-1)grid[selecaoY][selecaoX][selecaoZ].selecionado = false;selecaoX--;mudouSelecao = true;grid[selecaoY][selecaoX][selecaoZ].selecionado = true;}}
    if (key == GLFW_KEY_UP && action == GLFW_PRESS){if (selecaoY + 1 < TAM){if(selecaoY!=-1)grid[selecaoY][selecaoX][selecaoZ].selecionado = false;selecaoY++;mudouSelecao = true;grid[selecaoY][selecaoX][selecaoZ].selecionado = true;}}
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){if (selecaoY - 1 >= 0){if(selecaoY!=-1)grid[selecaoY][selecaoX][selecaoZ].selecionado = false;selecaoY--;mudouSelecao = true;grid[selecaoY][selecaoX][selecaoZ].selecionado = true;}}
    if (key == GLFW_KEY_PAGE_UP && action == GLFW_PRESS){if (selecaoZ + 1 < TAM){if(selecaoZ!=-1)grid[selecaoY][selecaoX][selecaoZ].selecionado = false;selecaoZ++;mudouSelecao = true;grid[selecaoY][selecaoX][selecaoZ].selecionado = true;}}
    if (key == GLFW_KEY_PAGE_DOWN && action == GLFW_PRESS){if (selecaoZ - 1 >= 0){if(selecaoZ!=-1)grid[selecaoY][selecaoX][selecaoZ].selecionado = false;selecaoZ--;mudouSelecao = true;grid[selecaoY][selecaoX][selecaoZ].selecionado = true;}}
    bool mudouCor = false;
    if (key == GLFW_KEY_C && action == GLFW_PRESS){if(selecaoX!=-1){int corAtual = grid[selecaoY][selecaoX][selecaoZ].corPos;if (corAtual < 5){corAtual++;}else{corAtual = 0;}mudouCor = true;grid[selecaoY][selecaoX][selecaoZ].corPos = corAtual;grid[selecaoY][selecaoX][selecaoZ].visivel = true;}}
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += 15.0f * deltaTime * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= 15.0f * deltaTime * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * 15.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * 15.0f * deltaTime;
}

void especificaVisualizacao()
{
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    GLuint loc = glGetUniformLocation(shaderID, "view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
}

void especificaProjecao()
{
    glm::mat4 proj = glm::perspective(glm::radians(fov), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
    GLuint loc = glGetUniformLocation(shaderID, "proj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
}

void transformaObjeto(float xpos, float ypos, float zpos, float xrot, float yrot, float zrot, float sx, float sy, float sz)
{
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(xpos, ypos, zpos));
    transform = glm::rotate(transform, glm::radians(xrot), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(yrot), glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, glm::radians(zrot), glm::vec3(0, 0, 1));
    transform = glm::scale(transform, glm::vec3(sx, sy, sz));
    GLuint loc = glGetUniformLocation(shaderID, "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(transform));
}

GLuint setupShader()
{
    GLint success;
    GLchar infoLog[512];
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        cout << "Vertex Shader error:\n" << infoLog << endl;
    }
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        cout << "Fragment Shader error:\n" << infoLog << endl;
    }
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        cout << "Shader Program Linking error:\n" << infoLog << endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

GLuint setupGeometry()
{
    GLfloat vertices[] = {
        0.5, 0.5, 0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5,
        0.5, 0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5,
        -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5,
        0.5, -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5,
        -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, -0.5, 0.5,
        -0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5
    };
    GLuint VBO, vao;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &VBO);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return vao;
}

void setColor(GLuint shaderID, glm::vec4 cor)
{
    GLint loc = glGetUniformLocation(shaderID, "uColor");
    glUniform4f(loc, cor.r, cor.g, cor.b, cor.a);
}

void inicializarGradeVoxel(int tamanho)
{
    TAM = tamanho;
    grid = new Voxel **[TAM];
    for (int y = 0; y < TAM; y++)
    {
        grid[y] = new Voxel *[TAM];
        for (int x = 0; x < TAM; x++)
        {
            grid[y][x] = new Voxel[TAM];
        }
    }
    for (int y = 0, yPos = -TAM / 2; y < TAM; y++, yPos += 1.0f)
    {
        for (int x = 0, xPos = -TAM / 2; x < TAM; x++, xPos += 1.0f)
        {
            for (int z = 0, zPos = -TAM / 2; z < TAM; z++, zPos += 1.0f)
            {
                grid[y][x][z].pos = glm::vec3(xPos, yPos, zPos);
                grid[y][x][z].corPos = 0;
                grid[y][x][z].fatorEscala = 0.98f;
                grid[y][x][z].visivel = false;
                grid[y][x][z].selecionado = false;
            }
        }
    }
}

void reiniciarGradeVoxel(int novoTamanho)
{
    if (grid != nullptr)
    {
        for (int y = 0; y < TAM; y++)
        {
            for (int x = 0; x < TAM; x++)
            {
                delete[] grid[y][x];
            }
            delete[] grid[y];
        }
        delete[] grid;
        grid = nullptr;
    }

    inicializarGradeVoxel(novoTamanho);

    selecaoX = 0;
    selecaoY = 0;
    selecaoZ = novoTamanho > 0 ? novoTamanho - 1 : 0;
    if (novoTamanho > 0) {
        grid[selecaoY][selecaoX][selecaoZ].selecionado = true;
    }

    cout << "Grade reiniciada com tamanho: " << novoTamanho << "x" << novoTamanho << "x" << novoTamanho << endl;
}

void atualizarSelecaoComMouse()
{
    glm::mat4 proj = glm::perspective(glm::radians(fov), (float)WIDTH / HEIGHT, 0.1f, 1000.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    float x = (2.0f * xpos) / WIDTH - 1.0f;
    float y = 1.0f - (2.0f * ypos) / HEIGHT;
    glm::vec4 ray_clip = glm::vec4(x, y, -1.0, 1.0);
    glm::vec4 ray_eye = glm::inverse(proj) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
    glm::vec3 ray_wor = glm::normalize(glm::vec3(glm::inverse(view) * ray_eye));

    if (selecaoX != -1) {
        grid[selecaoY][selecaoX][selecaoZ].selecionado = false;
    }

    glm::vec3 rayOrigin = cameraPos;
    bool hit = false;
    int hitX = -1, hitY = -1, hitZ = -1; 
    int placeX = -1, placeY = -1, placeZ = -1;

    for (float step = 0; step < 200.0f; step += 0.5f)
    {
        glm::vec3 currentPoint = rayOrigin + ray_wor * step;
        int gridX = static_cast<int>(round(currentPoint.x + (TAM / 2.0f)));
        int gridY = static_cast<int>(round(currentPoint.y + (TAM / 2.0f)));
        int gridZ = static_cast<int>(round(currentPoint.z + (TAM / 2.0f)));

        if (gridX >= 0 && gridX < TAM && gridY >= 0 && gridY < TAM && gridZ >= 0 && gridZ < TAM)
        {
            if (grid[gridY][gridX][gridZ].visivel) {
                glm::vec3 prevPoint = rayOrigin + ray_wor * (step - 0.5f);
                placeX = static_cast<int>(round(prevPoint.x + (TAM / 2.0f)));
                placeY = static_cast<int>(round(prevPoint.y + (TAM / 2.0f)));
                placeZ = static_cast<int>(round(prevPoint.z + (TAM / 2.0f)));

                hitX = gridX;
                hitY = gridY;
                hitZ = gridZ;

                hit = true;
                break;
            }
        }
    }
    
    if (hit) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
             selecaoX = hitX;
             selecaoY = hitY;
             selecaoZ = hitZ;
        } 
        else {
             selecaoX = placeX;
             selecaoY = placeY;
             selecaoZ = placeZ;
        }

        if(selecaoX >= 0 && selecaoX < TAM && selecaoY >= 0 && selecaoY < TAM && selecaoZ >= 0 && selecaoZ < TAM) {
             grid[selecaoY][selecaoX][selecaoZ].selecionado = true;
        } else {
            selecaoX = -1;
        }
    } else {
        selecaoX = -1;
    }
}


void renderHUD(GLuint shaderID, GLuint VAO, const Voxel& selectedVoxel)
{
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    if (depthTestEnabled) {
        glDisable(GL_DEPTH_TEST);
    }
    glm::mat4 ortho_projection = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "proj"), 1, GL_FALSE, glm::value_ptr(ortho_projection));
    glm::mat4 viewHUD = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, glm::value_ptr(viewHUD));
    
    float quadSize = 30.0f;
    float padding = 10.0f;
    float startX = 20.0f;
    float startY = 20.0f;
    float selectionBoxSize = quadSize + 10.0f;
    glBindVertexArray(VAO);
    for (int i = 1; i < sizeof(colorList) / sizeof(glm::vec4); i++)
    {
        float currentX = startX + (i-1) * (quadSize + padding);
        
        bool isCurrentDrawingColor = (i == corDesenhoAtual);
        bool isSelectedBlockColor = (!modoDesenho && i == selectedVoxel.corPos);

        if (isCurrentDrawingColor || isSelectedBlockColor)
        {
            setColor(shaderID, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
            transformaObjeto(currentX + quadSize / 2.0f, startY + quadSize / 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, selectionBoxSize, selectionBoxSize, 1.0f);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        setColor(shaderID, colorList[i]);
        transformaObjeto(currentX + quadSize / 2.0f, startY + quadSize / 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, quadSize, quadSize, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindVertexArray(0);
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
}


const glm::vec4 btnSalvarRect(WIDTH / 2.0f - 100.0f, HEIGHT / 2.0f + 30.0f, 200.0f, 50.0f);
const glm::vec4 btnCarregarRect(WIDTH / 2.0f - 100.0f, HEIGHT / 2.0f - 30.0f, 200.0f, 50.0f);
const glm::vec4 btnSairRect(WIDTH / 2.0f - 100.0f, HEIGHT / 2.0f - 90.0f, 200.0f, 50.0f);

void renderMenu()
{
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    if (depthTestEnabled) glDisable(GL_DEPTH_TEST);

    glm::mat4 ortho_projection = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "proj"), 1, GL_FALSE, glm::value_ptr(ortho_projection));
    glm::mat4 viewHUD = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, glm::value_ptr(viewHUD));

    glBindVertexArray(VAO);

    setColor(shaderID, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
    transformaObjeto(WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, WIDTH, HEIGHT, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    float highlightSize = 10.0f;
    glm::vec4 corBotao;

    corBotao = glm::vec4(0.2f, 0.8f, 0.2f, 1.0f);
    if (menuBotaoPressionado == 0) {
        corBotao = glm::vec4(0.1f, 0.5f, 0.1f, 1.0f);
    }
    if (menuSelecao == 0 && menuBotaoPressionado != 0) {
        setColor(shaderID, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
        transformaObjeto(btnSalvarRect.x + btnSalvarRect.z / 2.0f, btnSalvarRect.y + btnSalvarRect.w / 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, btnSalvarRect.z + highlightSize, btnSalvarRect.w + highlightSize, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    setColor(shaderID, corBotao);
    transformaObjeto(btnSalvarRect.x + btnSalvarRect.z / 2.0f, btnSalvarRect.y + btnSalvarRect.w / 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, btnSalvarRect.z, btnSalvarRect.w, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    corBotao = glm::vec4(0.2f, 0.2f, 0.8f, 1.0f);
    if (menuBotaoPressionado == 1) {
        corBotao = glm::vec4(0.1f, 0.1f, 0.5f, 1.0f);
    }
    if (menuSelecao == 1 && menuBotaoPressionado != 1) {
        setColor(shaderID, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
        transformaObjeto(btnCarregarRect.x + btnCarregarRect.z / 2.0f, btnCarregarRect.y + btnCarregarRect.w / 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, btnCarregarRect.z + highlightSize, btnCarregarRect.w + highlightSize, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    setColor(shaderID, corBotao);
    transformaObjeto(btnCarregarRect.x + btnCarregarRect.z / 2.0f, btnCarregarRect.y + btnCarregarRect.w / 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, btnCarregarRect.z, btnCarregarRect.w, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    corBotao = glm::vec4(0.8f, 0.2f, 0.2f, 1.0f);
    if (menuBotaoPressionado == 2) {
        corBotao = glm::vec4(0.5f, 0.1f, 0.1f, 1.0f);
    }
    if (menuSelecao == 2 && menuBotaoPressionado != 2) {
        setColor(shaderID, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
        transformaObjeto(btnSairRect.x + btnSairRect.z / 2.0f, btnSairRect.y + btnSairRect.w / 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, btnSairRect.z + highlightSize, btnSairRect.w + highlightSize, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    setColor(shaderID, corBotao);
    transformaObjeto(btnSairRect.x + btnSairRect.z / 2.0f, btnSairRect.y + btnSairRect.w / 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, btnSairRect.z, btnSairRect.w, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glBindVertexArray(0);
    if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (menuAberto) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                if (menuSelecao != -1) menuBotaoPressionado = menuSelecao;
            } else if (action == GLFW_RELEASE) {
                if (menuBotaoPressionado == menuSelecao && menuBotaoPressionado != -1) {
                    ativarOpcaoMenu();
                }
                menuBotaoPressionado = -1;
            }
        }
    } 
    else if (modoDesenho && action == GLFW_PRESS)
    {
        if (selecaoX != -1)
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT)
            {
                grid[selecaoY][selecaoX][selecaoZ].visivel = true;
                grid[selecaoY][selecaoX][selecaoZ].corPos = corDesenhoAtual;
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT)
            {
                grid[selecaoY][selecaoX][selecaoZ].visivel = false;
            }
        }
    }
}


int main()
{
    glfwInit();
    window = glfwCreateWindow(WIDTH, HEIGHT, "Voxel Editor", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    shaderID = setupShader();
    VAO = setupGeometry();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    inicializarGradeVoxel(101);

    ultimoSelecaoX = selecaoX = 0;
    ultimoSelecaoY = selecaoY = 0;
    ultimoSelecaoZ = selecaoZ = TAM - 1;
    grid[selecaoY][selecaoX][selecaoZ].selecionado = true;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        if (!menuAberto && !modoDesenho)
        {
            processInput(window);
        }
        
        if (menuAberto)
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            ypos = HEIGHT - ypos;

            if (xpos >= btnSalvarRect.x && xpos <= btnSalvarRect.x + btnSalvarRect.z &&
                ypos >= btnSalvarRect.y && ypos <= btnSalvarRect.y + btnSalvarRect.w)
            {
                menuSelecao = 0;
            }
            else if (xpos >= btnCarregarRect.x && xpos <= btnCarregarRect.x + btnCarregarRect.z &&
                     ypos >= btnCarregarRect.y && ypos <= btnCarregarRect.y + btnCarregarRect.w)
            {
                menuSelecao = 1;
            }
            else if (xpos >= btnSairRect.x && xpos <= btnSairRect.x + btnSairRect.z &&
                     ypos >= btnSairRect.y && ypos <= btnSairRect.y + btnSairRect.w)
            {
                menuSelecao = 2;
            }
        }
        
        if (modoDesenho && !menuAberto)
        {
            atualizarSelecaoComMouse();
        }
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderID);

        especificaVisualizacao();
        especificaProjecao();

        glBindVertexArray(VAO);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        setColor(shaderID, glm::vec4(1.0f, 1.0f, 1.0f, 0.1f));
        transformaObjeto(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, TAM, TAM, TAM);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        for (int x = 0; x < TAM; x++)
        {
            for (int y = 0; y < TAM; y++)
            {
                for (int z = 0; z < TAM; z++)
                {
                    if (grid[y][x][z].selecionado)
                    {
                        setColor(shaderID, colorList[grid[y][x][z].corPos] + 0.3f);
                    }
                    else
                    {
                        setColor(shaderID, colorList[grid[y][x][z].corPos]);
                    }
                    if (grid[y][x][z].visivel || grid[y][x][z].selecionado)
                    {
                        float fatorEscala = grid[y][x][z].fatorEscala;
                        transformaObjeto(grid[y][x][z].pos.x, grid[y][x][z].pos.y, grid[y][x][z].pos.z, 0.0f, 0.0f, 0.0f, fatorEscala, fatorEscala, fatorEscala);
                        glDrawArrays(GL_TRIANGLES, 0, 36);
                    }
                }
            }
        }
        
        if (menuAberto)
        {
            renderMenu();
        }
        else
        {
            Voxel dummyVoxel;
            dummyVoxel.corPos = -1; 
            if (selecaoX != -1)
            {
                renderHUD(shaderID, VAO, modoDesenho ? dummyVoxel : grid[selecaoY][selecaoX][selecaoZ]);
            }
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}