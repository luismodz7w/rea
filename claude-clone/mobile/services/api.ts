import axios, { AxiosInstance } from 'axios';
import AsyncStorage from '@react-native-async-storage/async-storage';

const API_BASE_URL = process.env.REACT_APP_API_URL || 'http://localhost:3000';

export interface Message {
  id: string;
  role: 'user' | 'assistant';
  content: string;
  created_at?: string;
}

export interface Conversation {
  id: string;
  title: string;
  created_at: string;
  updated_at: string;
  messages?: Message[];
}

class APIClient {
  private client: AxiosInstance;
  private baseUrl = API_BASE_URL;

  constructor() {
    this.client = axios.create({
      baseURL: this.baseUrl,
      timeout: 60000,
    });
  }

  async init() {
    const token = await AsyncStorage.getItem('auth_token');
    if (token) {
      this.client.defaults.headers.common['Authorization'] = `Bearer ${token}`;
    }
  }

  // Conversations
  async createConversation(title?: string): Promise<Conversation> {
    const response = await this.client.post('/api/chat/conversations', { title });
    return response.data;
  }

  async getConversations(): Promise<Conversation[]> {
    const response = await this.client.get('/api/chat/conversations');
    return response.data;
  }

  async getConversation(id: string): Promise<Conversation> {
    const response = await this.client.get(`/api/chat/conversations/${id}`);
    return response.data;
  }

  async deleteConversation(id: string): Promise<void> {
    await this.client.delete(`/api/chat/conversations/${id}`);
  }

  // Messages
  async sendMessage(
    conversationId: string,
    content: string,
    model: string = 'mistral'
  ): Promise<any> {
    const response = await this.client.post(
      `/api/chat/conversations/${conversationId}/messages`,
      { content, model }
    );
    return response.data;
  }

  async *sendMessageStream(
    conversationId: string,
    content: string,
    model: string = 'mistral'
  ) {
    const response = await this.client.post(
      `/api/chat/conversations/${conversationId}/messages/stream`,
      { content, model },
      { responseType: 'stream' }
    );

    for await (const chunk of response.data) {
      const line = chunk.toString().trim();
      if (line) {
        yield JSON.parse(line);
      }
    }
  }

  // Models
  async getAvailableModels(): Promise<any[]> {
    const response = await this.client.get('/api/models');
    return response.data.models;
  }

  // Health check
  async checkHealth(): Promise<boolean> {
    try {
      const response = await this.client.get('/health');
      return response.status === 200;
    } catch {
      return false;
    }
  }

  async checkLLMStatus(): Promise<any> {
    try {
      const response = await this.client.get('/api/llm/status');
      return response.data;
    } catch (error) {
      return { status: 'unavailable' };
    }
  }

  setBaseUrl(url: string) {
    this.baseUrl = url;
    this.client.defaults.baseURL = url;
  }
}

export const APIClient_instance = new APIClient();
export { APIClient_instance as APIClient };
