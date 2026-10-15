import axios from 'axios';

const OLLAMA_URL = process.env.OLLAMA_URL || 'http://localhost:11434';
const DEFAULT_MODEL = process.env.LLM_MODEL || 'mistral';

export interface OllamaMessage {
  role: 'user' | 'assistant' | 'system';
  content: string;
}

export interface OllamaResponse {
  model: string;
  created_at: string;
  message: OllamaMessage;
  done: boolean;
  total_duration?: number;
  load_duration?: number;
  prompt_eval_count?: number;
  eval_count?: number;
  eval_duration?: number;
}

class OllamaService {
  private baseUrl = OLLAMA_URL;
  private defaultModel = DEFAULT_MODEL;

  async checkHealth() {
    try {
      const response = await axios.get(`${this.baseUrl}/api/tags`, {
        timeout: 5000
      });
      return {
        status: 'healthy',
        models: response.data.models?.map((m: any) => m.name) || []
      };
    } catch (error) {
      return {
        status: 'unavailable',
        error: 'Cannot connect to Ollama'
      };
    }
  }

  async chat(messages: OllamaMessage[], model: string = this.defaultModel) {
    try {
      const response = await axios.post(
        `${this.baseUrl}/api/chat`,
        {
          model,
          messages,
          stream: false,
          options: {
            temperature: 0.7,
            top_p: 0.9,
            top_k: 40
          }
        },
        { timeout: 60000 }
      );

      return response.data as OllamaResponse;
    } catch (error: any) {
      throw new Error(`LLM Error: ${error.message}`);
    }
  }

  async *chatStream(messages: OllamaMessage[], model: string = this.defaultModel) {
    try {
      const response = await axios.post(
        `${this.baseUrl}/api/chat`,
        {
          model,
          messages,
          stream: true,
          options: {
            temperature: 0.7,
            top_p: 0.9,
            top_k: 40
          }
        },
        {
          timeout: 60000,
          responseType: 'stream'
        }
      );

      for await (const chunk of response.data) {
        const line = chunk.toString().trim();
        if (line) {
          yield JSON.parse(line) as OllamaResponse;
        }
      }
    } catch (error: any) {
      throw new Error(`LLM Stream Error: ${error.message}`);
    }
  }

  async getAvailableModels() {
    try {
      const response = await axios.get(`${this.baseUrl}/api/tags`);
      return response.data.models?.map((m: any) => ({
        name: m.name,
        modified_at: m.modified_at,
        size: m.size
      })) || [];
    } catch (error) {
      return [];
    }
  }
}

export const ollama = new OllamaService();
