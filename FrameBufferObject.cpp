#include "FrameBufferObject.h"


FrameBufferObject::FrameBufferObject()
{
}


FrameBufferObject::~FrameBufferObject()
{
}

void FrameBufferObject::bind(bool FBO_ENABLED){
	
}

void FrameBufferObject::unbind(bool FBO_ENABLED){

}

void FrameBufferObject::init(int width, int height){
	glGenTextures(1, &fboTextureID);
	glBindTexture(GL_TEXTURE_2D, fboTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTextureID, 0);
	glGenRenderbuffers(1, &rboID);
	//glGenTextures(1, &fbo.rboTextureID);
	//glBindTexture(1, fbo.rboTextureID);
	glBindRenderbuffer(GL_RENDERBUFFER, rboID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID);
	//auto b = glewGetErrorString(glGetError());
	GLenum e = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (e != GL_FRAMEBUFFER_COMPLETE)
		printf("There is a problem with the FBO\n");
	else{
		std::vector<string> shaders{ "FBOshader" };
		for each (string shader in shaders)
			fboShaders.push_back(new ShaderProgram("data/CrimeSceneV4/Shaders/" + shader + ".vert", "data/CrimeSceneV4/Shaders/" + shader + ".frag"));
		for each (ShaderProgram* program in fboShaders)
		{
			program->link();
			program->setUniformInt("frame_texture", 0);
			program->setUniformInt("font_texture", 1);
		}
	}
}

void FrameBufferObject::use(){
	glBindVertexArray(0);
	glEnableVertexAttribArray(0);                                                   // en vertex attribute 1
	glDisableVertexAttribArray(1);                                                  // disable vertex attribute 1
	glDisableVertexAttribArray(2);                                                  // disable vertex attribute 1
	glBindTexture(GL_TEXTURE_2D, fboTextureID);	
	std::vector<glm::vec2> verts;
	verts.push_back(glm::vec2(-1, -1));
	verts.push_back(glm::vec2(1, -1));
	verts.push_back(glm::vec2(1, 1));
	verts.push_back(glm::vec2(-1, 1));
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * 4, &verts[0]);                                                                 //geef aan dat de posities op deze locatie zitten
	glDrawArrays(GL_QUADS, 0, verts.size());
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisableVertexAttribArray(0);
}