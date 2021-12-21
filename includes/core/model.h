//ʹ��assimp��ȡobj/fbx,assimp֧�ֶ��ָ�ʽ��ֱ��ʹ��import���ص�scene�м���
//loadModel
// Vertices����ÿ��Vertex��ÿ��Vertex����position��normal��TexCoords��Tangent��Bitangent��Ϣ

#pragma once

#include <assimp/Importer.hpp>//����obj/fbx
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <core/mesh.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>



class Model
{
public:
    // model data 
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.

    vector<Mesh> meshes;
    // vector<Vertex> vertices : Position, Normal, TexCoords, Tangent, Bitangent
    // vector<unsigned int> indices ���е�faces���
    // vector<texture> textures ����texture��Ϣ
    //==============================================================================



    //directory������Ѱ��texture��ͼ��Ϣ��
    string directory;
    bool gammaCorrection;

    // constructor, expects a filepath to a 3D model.
    Model(string const& path = "", bool gamma = false) : gammaCorrection(gamma) {};
    ~Model() {};

    void Load(string const& path)
    {
        //ʹ��assimp����obj/fbx
        this->loadModel(path);

        //����������mesh��Ϣ
        //this->SaveModelData("resources/objects/mesh/teapot", "dat");

        //ֱ��ͨ�������mesh.dat/txt����ģ�Ͳ���Ⱦ
        //this->loadModelData("resources/objects/mesh/teapot", "dat");
    }

    // draws the model, and thus all its meshes
    void Draw(Shader& shader)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
        {
            meshes[i].Draw(shader); //����֧�ֶ��meshͬʱ����                  
        }
    }

    void SaveModelData(const string& path, const string& file_extension)
    {
        string mesh_name = path.substr(path.rfind("/") + 1);

        //savemodelʱ����Ҫ��loadmodel ��Ȼû��assimp��ȡ�Ĺ���
        for (unsigned int i = 0; i < meshes.size(); ++i)
        {
            //��ö�mesh��ÿ��mesh��mesh-path                
            string mesh_path = path + "/" + mesh_name + to_string(static_cast<long long>(i)) + "." + file_extension;
            cout << mesh_path << endl;

            const int row_max = meshes[i].indices.size();
            ofstream outputfile(mesh_path);
            if (outputfile.is_open()) {
                for (int row = 0; row < row_max; ++row) {
                    outputfile << meshes[i].indices[row] << " ";
                    outputfile << meshes[i].vertices[row].Position.x << " ";
                    outputfile << meshes[i].vertices[row].Position.y << " ";
                    outputfile << meshes[i].vertices[row].Position.z << " ";
                    outputfile << meshes[i].vertices[row].Normal.x << " ";
                    outputfile << meshes[i].vertices[row].Normal.y << " ";
                    outputfile << meshes[i].vertices[row].Normal.z << " ";
                    outputfile << meshes[i].vertices[row].TexCoords.x << " ";
                    outputfile << meshes[i].vertices[row].TexCoords.y << " ";

                    //��������е�normalΪ0�����
                    meshes[i].vertices[row].Tangent.x = isnan(meshes[i].vertices[row].Tangent.x) ? 0 : meshes[i].vertices[row].Tangent.x;
                    outputfile << meshes[i].vertices[row].Tangent.x << " ";
                    meshes[i].vertices[row].Tangent.y = isnan(meshes[i].vertices[row].Tangent.y) ? 0 : meshes[i].vertices[row].Tangent.y;
                    outputfile << meshes[i].vertices[row].Tangent.y << " ";
                    meshes[i].vertices[row].Tangent.z = isnan(meshes[i].vertices[row].Tangent.z) ? 0 : meshes[i].vertices[row].Tangent.z;
                    outputfile << meshes[i].vertices[row].Tangent.z << " ";

                    //��normalΪ0ʱ ���е�bitangentҲΪ0
                    if (meshes[i].vertices[row].Normal.x == 0 && meshes[i].vertices[row].Normal.y == 0 && meshes[i].vertices[row].Normal.z == 0)
                    {
                        meshes[i].vertices[row].Bitangent.x = 0;
                        meshes[i].vertices[row].Bitangent.y = 0;
                        meshes[i].vertices[row].Bitangent.z = 0;
                    }
                    outputfile << meshes[i].vertices[row].Bitangent.x << " ";
                    outputfile << meshes[i].vertices[row].Bitangent.y << " ";
                    outputfile << meshes[i].vertices[row].Bitangent.z << endl;

                }
            }
            else
            {
                cout << "error with save model file " << path << endl;
            }
            outputfile.close();
        }

        cout << "meshes size " << meshes.size() << " all saved" << endl;
    }


private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const& path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        //aiProcess:trangulate
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    void loadModelData(const string& path, const string& file_extension)
    {       
        string file_Dir = path + "/";
        string mesh_name = path.substr(path.rfind("/") + 1);

        //���һ���ļ������ж��ٸ�txt�ļ�
        unsigned int quantity = GetFilesQuantity(file_Dir.c_str(), file_extension.c_str());
        for (unsigned int i = 0; i < quantity; ++i)
        {
            string mesh_path = path + "/" + mesh_name + to_string(static_cast<long long>(i)) + "." + file_extension;
            meshes.push_back(processMeshData(mesh_path));
        }

    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }


    //��������������ԣ������浽�Լ��Ķ�����
    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // walk through each of the mesh's vertices
        //ÿ�ֵ�����ȡmesh��vertex��Ϣ�������ѹ�뵽vertices��
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;


            //������λ�� ���� ���� ͨ������mNumVertices�������е����ж��㣩
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            //assimp������λ���������mVertices
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;//��vertex��positionͨ��vector����

            // normals
            //assimp��normal�������mNormals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }

            // texture coordinates
            //assimp�������������mTextureCoords�����Դ��л��coords��tangent��bitangent
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }


        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        // �����ﶨ�������� Ҳ����indices
        //ÿ��face��һ��������,assimp����ÿ��������face��������һ���������ж���������Ҷ����˶���Ļ���˳��
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }


        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
        //  
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // return a mesh object created from the extracted mesh data
        //ȫ����vertices��indices��textures������� ����mesh
        return Mesh(vertices, indices, textures);
    }

    Mesh processMeshData(string const& path)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        int tmp;
        float tmp1, tmp2, tmp3;
        glm::vec3 position, normal, tangent, bitangent;
        glm::vec2 texcoords;

        string file_extension = path.substr(path.rfind(".") + 1);
        ifstream infile(path, file_extension == "dat" ? ios::in | ios::binary : ios::in);//�����txt��ѡ��in ����ѡ��binary

        if (!infile.is_open())
        {
            cout << "open failed" << endl;
            exit(1);
        }

        //while (!file.eof() && file.peek() != EOF) ��ֹ���һ�п������
        while (infile >> tmp) 
        {
            indices.push_back(tmp);
            infile >> tmp1 >> tmp2 >> tmp3;
            position = glm::vec3(tmp1, tmp2, tmp3);
            infile >> tmp1 >> tmp2 >> tmp3;
            normal = glm::vec3(tmp1, tmp2, tmp3);
            infile >> tmp1 >> tmp2;
            texcoords = glm::vec2(tmp1, tmp2);
            infile >> tmp1 >> tmp2 >> tmp3;
            tangent = glm::vec3(tmp1, tmp2, tmp3);
            infile >> tmp1 >> tmp2 >> tmp3;
            bitangent = glm::vec3(tmp1, tmp2, tmp3);
            vertices.push_back({ position, normal, texcoords, tangent, bitangent });
        }
        infile.close();

        return Mesh(vertices, indices, textures);
    }



    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            //�ж�������û�б����ع������ع���skip��û���ع��ͼ���
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }
};



