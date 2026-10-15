import express, { Router } from 'express';
import { v4 as uuidv4 } from 'uuid';
import { getDb } from '../db/init';
import { ollama } from '../services/ollama';

export const chatRouter = Router();

// Create new conversation
chatRouter.post('/conversations', async (req, res) => {
  try {
    const db = await getDb();
    const id = uuidv4();
    const title = req.body.title || 'New Chat';

    await db.run(
      'INSERT INTO conversations (id, title) VALUES (?, ?)',
      [id, title]
    );

    res.json({ id, title, created_at: new Date().toISOString() });
  } catch (error: any) {
    res.status(500).json({ error: error.message });
  }
});

// Get conversations list
chatRouter.get('/conversations', async (req, res) => {
  try {
    const db = await getDb();
    const conversations = await db.all(
      'SELECT * FROM conversations WHERE archived = 0 ORDER BY updated_at DESC'
    );
    res.json(conversations);
  } catch (error: any) {
    res.status(500).json({ error: error.message });
  }
});

// Get conversation and messages
chatRouter.get('/conversations/:id', async (req, res) => {
  try {
    const db = await getDb();
    const conversation = await db.get(
      'SELECT * FROM conversations WHERE id = ?',
      [req.params.id]
    );

    if (!conversation) {
      return res.status(404).json({ error: 'Conversation not found' });
    }

    const messages = await db.all(
      'SELECT * FROM messages WHERE conversation_id = ? ORDER BY created_at ASC',
      [req.params.id]
    );

    res.json({ ...conversation, messages });
  } catch (error: any) {
    res.status(500).json({ error: error.message });
  }
});

// Send message and get response
chatRouter.post('/conversations/:id/messages', async (req, res) => {
  try {
    const { content, model = 'mistral' } = req.body;
    const conversationId = req.params.id;
    const db = await getDb();

    if (!content) {
      return res.status(400).json({ error: 'Message content required' });
    }

    // Save user message
    const userMessageId = uuidv4();
    await db.run(
      'INSERT INTO messages (id, conversation_id, role, content) VALUES (?, ?, ?, ?)',
      [userMessageId, conversationId, 'user', content]
    );

    // Get conversation history
    const messages = await db.all(
      'SELECT role, content FROM messages WHERE conversation_id = ? ORDER BY created_at ASC',
      [conversationId]
    );

    // Get AI response (without censorship)
    const response = await ollama.chat(messages, model);

    // Save assistant message
    const assistantMessageId = uuidv4();
    await db.run(
      'INSERT INTO messages (id, conversation_id, role, content) VALUES (?, ?, ?, ?)',
      [assistantMessageId, conversationId, 'assistant', response.message.content]
    );

    // Update conversation timestamp
    await db.run(
      'UPDATE conversations SET updated_at = CURRENT_TIMESTAMP WHERE id = ?',
      [conversationId]
    );

    res.json({
      userMessage: { id: userMessageId, role: 'user', content },
      assistantMessage: {
        id: assistantMessageId,
        role: 'assistant',
        content: response.message.content
      },
      model: response.model,
      stats: {
        total_duration: response.total_duration,
        eval_count: response.eval_count,
        prompt_eval_count: response.prompt_eval_count
      }
    });
  } catch (error: any) {
    res.status(500).json({ error: error.message });
  }
});

// Stream response
chatRouter.post('/conversations/:id/messages/stream', async (req, res) => {
  try {
    const { content, model = 'mistral' } = req.body;
    const conversationId = req.params.id;
    const db = await getDb();

    if (!content) {
      return res.status(400).json({ error: 'Message content required' });
    }

    // Save user message
    const userMessageId = uuidv4();
    await db.run(
      'INSERT INTO messages (id, conversation_id, role, content) VALUES (?, ?, ?, ?)',
      [userMessageId, conversationId, 'user', content]
    );

    // Get conversation history
    const messages = await db.all(
      'SELECT role, content FROM messages WHERE conversation_id = ? ORDER BY created_at ASC',
      [conversationId]
    );

    res.setHeader('Content-Type', 'application/x-ndjson');

    let fullResponse = '';
    const assistantMessageId = uuidv4();

    for await (const chunk of ollama.chatStream(messages, model)) {
      fullResponse += chunk.message.content;
      res.write(JSON.stringify({
        id: assistantMessageId,
        chunk: chunk.message.content,
        done: chunk.done
      }) + '\n');
    }

    // Save full assistant message
    await db.run(
      'INSERT INTO messages (id, conversation_id, role, content) VALUES (?, ?, ?, ?)',
      [assistantMessageId, conversationId, 'assistant', fullResponse]
    );

    // Update conversation timestamp
    await db.run(
      'UPDATE conversations SET updated_at = CURRENT_TIMESTAMP WHERE id = ?',
      [conversationId]
    );

    res.end();
  } catch (error: any) {
    res.status(500).json({ error: error.message });
  }
});

// Delete conversation
chatRouter.delete('/conversations/:id', async (req, res) => {
  try {
    const db = await getDb();
    await db.run(
      'UPDATE conversations SET archived = 1 WHERE id = ?',
      [req.params.id]
    );
    res.json({ success: true });
  } catch (error: any) {
    res.status(500).json({ error: error.message });
  }
});
