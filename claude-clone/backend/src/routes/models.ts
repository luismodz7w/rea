import { Router } from 'express';
import { ollama } from '../services/ollama';

export const modelRouter = Router();

// List available models
modelRouter.get('/', async (req, res) => {
  try {
    const models = await ollama.getAvailableModels();
    res.json({ models });
  } catch (error: any) {
    res.status(500).json({ error: error.message });
  }
});

// Check model availability
modelRouter.get('/:name', async (req, res) => {
  try {
    const models = await ollama.getAvailableModels();
    const model = models.find(m => m.name === req.params.name);

    if (!model) {
      return res.status(404).json({ error: 'Model not found' });
    }

    res.json(model);
  } catch (error: any) {
    res.status(500).json({ error: error.message });
  }
});
