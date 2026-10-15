import express from 'express';
import cors from 'cors';
import { initDatabase } from './db/init';
import { chatRouter } from './routes/chat';
import { modelRouter } from './routes/models';
import { fileRouter } from './routes/files';
import { ollama } from './services/ollama';

const app = express();
const PORT = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(express.json({ limit: '50mb' }));

// Initialize database
initDatabase();

// Routes
app.use('/api/chat', chatRouter);
app.use('/api/models', modelRouter);
app.use('/api/files', fileRouter);

// Health check
app.get('/health', (req, res) => {
  res.json({ status: 'ok', timestamp: new Date().toISOString() });
});

// LLM health check
app.get('/api/llm/status', async (req, res) => {
  try {
    const status = await ollama.checkHealth();
    res.json(status);
  } catch (error) {
    res.status(503).json({
      status: 'unavailable',
      error: 'Ollama service not responding'
    });
  }
});

app.listen(PORT, () => {
  console.log(`Claude Clone Backend running on port ${PORT}`);
  console.log(`Environment: ${process.env.NODE_ENV || 'development'}`);
  console.log(`Ollama URL: ${process.env.OLLAMA_URL || 'http://localhost:11434'}`);
});
